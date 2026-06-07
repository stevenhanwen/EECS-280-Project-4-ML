#include <cmath>
#include <iostream>
#include <fstream>
#include "csvstream.hpp"
#include <vector>
#include <set>
#include <map>
using namespace std;

class Classifier
{
private:
    int total_training_posts;

    // Use this set to keep track of the unique words that appear during training. 
    set<string> vocabulary_set;

    // A map with key as label and number of posts as value
    map<string, int> label_map;

    // A map with label as key and the sets of words that occurs in the label as value
    // This is used for the printed statements if the classifier is only trained. 
    map<string, set<string>> label_words_map;

    // This map is used for finding the number of times 
    // a word appears in posts for a particular label.
    // The key is the word and the value is another map with label as key 
    // and number of appearences of the word as value. 
    map<string, map<string, int>> word_count_map;

    // Create a non duplicate set of words from a vector of words
    set<string> create_word_set(const vector<string> &words)
    {
        std::set<string> word_set;
        // Each time a word is seen, add 1 to its entry in
        // the map. If it wasn't there, make a 0
        // placeholder and then immediately add 1 to that
        for (auto &word : words)
        {
            word_set.insert(word);
        }

        return word_set;
    }

    // Returns the number of training posts with label that contain word
    int find_num_posts_label_and_word(const string &word, const string &label)
    {   
        // Use an iterator to find the word in the map
        auto it1 = word_count_map.find(word);
        if (it1 != word_count_map.end())
        {   
            // Use another iterator to find a label for the word
            auto it2 = word_count_map[word].find(label);
            if (it2 != word_count_map[word].end())
            {
                return word_count_map[word][label]; 
            }
        
        }

        // If either is not found, then return 0 posts with that word given a label. 
        return 0; 
    }

    // Find the total number of training posts that contain the word
    int find_num_posts_with_word(const string &word)
    {   

        int total = 0; 
        // Use an iterator to find the word in the map
        auto it1 = word_count_map.find(word);
        if (it1 != word_count_map.end())
        {   

            // Go through every label and sum the number of posts that has that word. 
            for (auto& key_value_pair : word_count_map[word])
            {
                total += word_count_map[word][key_value_pair.first]; 
            }
                
        }

        return total; 
    }

    // Calculate the log-likelihood probability of a word given label
    double log_probability_word_given_label(const string &word, const string &label)
    {
        int numerator = find_num_posts_label_and_word(word, label);
        if (numerator != 0)
        {
            return log(static_cast<double>(numerator) / label_map[label]);
        }

        numerator = find_num_posts_with_word(word);
        if (numerator != 0)
        {
            return log(static_cast<double>(numerator) / total_training_posts);
        }

        return log(static_cast<double>(1) / total_training_posts);
    }

    // The log-prior probability of label and is a reflection of how common it is.
    double label_probability(const string &label)
    {
        return log(static_cast<double>(label_map[label]) / total_training_posts);
    }

    // This calculates the log probability of a post given a label
    // Takes in the set of words of the post
    double log_probability_post_given_label(const set<string> &word_set,
                                            const string &label)
    {
        double total_probability = label_probability(label);
        for (auto &word : word_set)
        {
            total_probability += log_probability_word_given_label(word, label);
        }

        return total_probability;
    }

    // Return the predicted label for a set of words and its probability value
    pair<string, double> predict_label(const set<string> &word_set)
    {
        // First value is the predicted label, second value is the calculated probability
        pair<string, double> prediction_pair;
        // This is the probabilities for a post given each label. 
        map<string, double> label_probabilities;
        for (auto &key_value_pair : label_map)
        {
            string label = key_value_pair.first;
            double probability = log_probability_post_given_label(word_set, label);
            label_probabilities[label] = probability;
        }

        // Use a for loop to find the highest probability calculated
        prediction_pair.first = label_probabilities.begin()->first;
        prediction_pair.second = label_probabilities.begin()->second;
        for (auto &key_value_pair : label_probabilities)
        {
            string label = key_value_pair.first;
            if (label_probabilities[label] > prediction_pair.second)
            {
                prediction_pair.first = label;
                prediction_pair.second = label_probabilities[label];
            }
        }

        return prediction_pair;
    }

public:
    Classifier()
    {
        total_training_posts = 0;
    }

    void print_training_info()
    {
        cout << "classes:" << endl;
        for (auto &key_value_pair : label_map)
        {
            cout << "  " << key_value_pair.first << ", ";
            cout << key_value_pair.second << " examples, ";
            cout << "log-prior = " << label_probability(key_value_pair.first) << endl;
        }

        cout << "classifier parameters:" << endl;
        for (auto &key_value_pair : label_words_map)
        {
            string label = key_value_pair.first;
            for (auto &word : key_value_pair.second)
            {
                cout << "  " << label << ":";
                cout << word << ", count = ";
                cout << find_num_posts_label_and_word(word, label) << ", ";
                cout << "log-likelihood = ";
                cout << log_probability_word_given_label(word, label) << endl;
            }
        }
        cout << endl;
    }

    void make_predictions(csvstream &test_csv_in)
    {
        cout << "trained on " << total_training_posts << " examples" << endl;
        cout << endl;

        int total_posts = 0;
        int predicted_correctly = 0;
        // Rows have key = column name, value = cell datum
        map<string, string> row;
        cout << "test data:" << endl;
        while (test_csv_in >> row)
        {
            // Get label and update num training posts with that label;
            string label = row["tag"];
            string content = row["content"];

            // First need to create a vector of words in the content
            vector<string> words_in_post;
            string curr_word = "";
            for (size_t i = 0; i < content.size(); ++i)
            {
                char curr_char = content.at(i);
                if (curr_char == ' ')
                {
                    words_in_post.push_back(curr_word);
                    curr_word = "";
                }
                else
                {
                    curr_word += curr_char;
                }
            }
            // Need to add the last word
            if (curr_word != "")
            {
                words_in_post.push_back(curr_word);
            }

            // Create the set of words
            set<string> word_set = create_word_set(words_in_post);

            pair<string, double> prediction_pair = predict_label(word_set);

            cout << "  correct = " << label << ", predicted = " << prediction_pair.first;
            cout << ", log-probability score = " << prediction_pair.second << endl;
            cout << "  content = " << content << endl;
            cout << endl;

            total_posts += 1;
            if (label == prediction_pair.first)
            {
                predicted_correctly += 1;
            }
        }

        cout << "performance: " << predicted_correctly << " / " << total_posts;
        cout << " posts predicted correctly" << endl;
    }

    Classifier(csvstream &train_csv_in, bool exists_test_file)
    {
        if (!exists_test_file)
        {
            cout << "training data:" << endl;
        }
        total_training_posts = 0;

        // Rows have key = column name, value = cell datum
        map<string, string> row;
        while (train_csv_in >> row)
        {
            // Get label and update num training posts with that label;
            string label = row["tag"];
            string content = row["content"];

            if (!exists_test_file)
            {
                cout << "  label = " << label << ", ";
                cout << "content = " << content << endl;
            }

            // First need to create a vector of words in the content
            vector<string> words_in_post;
            string curr_word = "";
            for (size_t i = 0; i < content.size(); ++i)
            {
                char curr_char = content.at(i);
                if (curr_char == ' ')
                {
                    words_in_post.push_back(curr_word);
                    vocabulary_set.insert(curr_word);
                    label_words_map[label].insert(curr_word);
                    curr_word = "";
                }
                else
                {
                    curr_word += curr_char;
                }
            }
            // Need to add the last word
            if (curr_word != "")
            {
                words_in_post.push_back(curr_word);
                vocabulary_set.insert(curr_word);
                label_words_map[label].insert(curr_word);
            }

            // Create the set of words
            set<string> word_set = create_word_set(words_in_post);
            for (const string &word : word_set)
            {
                word_count_map[word][label] += 1;
            }
           
            label_map[label] += 1; 
            total_training_posts += 1;
        }

        if (!exists_test_file)
        {
            cout << "trained on " << total_training_posts << " examples" << endl;
            cout << "vocabulary size = " << vocabulary_set.size() << endl;
            cout << endl;
        }
    }
};

int main(int argc, char *argv[])
{
    cout.precision(3);

    if (argc != 2 && argc != 3)
    {
        cout << "Usage: classifier.exe TRAIN_FILE [TEST_FILE]" << endl;
        return 1;
    }

    bool exists_test_file = false;
    if (argc == 3)
    {
        exists_test_file = true;
    }

    string train_file_name = argv[1];
    string test_file_name = "";
    if (exists_test_file)
    {
        test_file_name = argv[2];
    }

    try
    {
        csvstream csvtrain(train_file_name);
        Classifier classifier(csvtrain, exists_test_file);
        if (!exists_test_file)
        {
            classifier.print_training_info();
            return 0;
        }

        try
        {
            csvstream csvtest(test_file_name);
            classifier.make_predictions(csvtest);
        }
        catch (const csvstream_exception &e)
        {
            cerr << e.what() << "\n";
            return 1;
        }
    }
    catch (const csvstream_exception &e)
    {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}