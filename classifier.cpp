#include <cmath>
#include <iostream>
#include <fstream>
#include "csvstream.hpp"
#include <vector>
#include <set>
#include <map>
using namespace std;

struct Post
{
    string content;
    set<string> words;
    string label;
};

class Classifier
{
private:
    int total_training_posts;
    set<string> vocabulary_set;
    // A map with key as label and posts vector as value
    map<string, vector<Post>> label_map;
    // A map with key as label and the sets of words that occurs in the label as value
    map<string, set<string>> label_words_map;

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
    int find_num_posts_label_and_word(string word, string label)
    {
        int total = 0;
        for (size_t i = 0; i < label_map[label].size(); ++i)
        {
            const auto &post_word_set = label_map[label][i].words;
            auto it = post_word_set.find(word);
            if (it != post_word_set.end())
            {
                total += 1;
            }
        }

        return total;
    }

    // Find the total number of training posts that contain the word
    int find_num_posts_with_word(string word)
    {
        int total = 0;
        // Go through each key value pair in the map,
        // The key is the label name, the value is the vector of posts
        for (const auto &key_value_pair : label_map)
        {
            // Set reference variable to the posts vector
            const auto &posts_vector = key_value_pair.second;
            for (size_t i = 0; i < posts_vector.size(); ++i)
            {
                // Set reference variable to the words_set of a post
                const auto &post_word_set = posts_vector[i].words;
                // Look for the word in that set and update the total
                auto it = post_word_set.find(word);
                if (it != post_word_set.end())
                {
                    total += 1;
                }
            }
        }

        return total;
    }

    // Calculate the log-likelihood probability of a word given label
    double log_probability_word_given_label(string word, string label)
    {
        int numerator = find_num_posts_label_and_word(word, label);
        if (numerator != 0)
        {
            return log(static_cast<double>(numerator) / label_map[label].size());
        }

        numerator = find_num_posts_with_word(word);
        if (numerator != 0)
        {
            return log(static_cast<double>(numerator) / total_training_posts);
        }

        return log(static_cast<double>(1) / total_training_posts);
    }

    // The log-prior probability of label and is a reflection of how common it is.
    double label_probability(string label)
    {
        return log(static_cast<double>(label_map[label].size()) / total_training_posts);
    }

    // This calculates the log probability of a post given a label
    // Takes in the set of words of the post
    double log_probability_post_given_label(set<string> word_set, string label)
    {
        double total_probability = label_probability(label);
        for (auto &word : word_set)
        {
            total_probability += log_probability_word_given_label(word, label);
        }

        return total_probability;
    }

    pair<string, double> predict_label(set<string> word_set)
    {   
        pair<string, double> prediction_pair;
        for (auto& key_value_pair : label_map)
        {
            string label = key_value_pair.first; 
            double probability = log_probability_post_given_label(word_set, label); 
            
        }
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
            cout << key_value_pair.second.size() << " examples, ";
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
            words_in_post.push_back(curr_word);
            vocabulary_set.insert(curr_word);
            label_words_map[label].insert(curr_word);

            // Create the set of words
            set<string> word_set = create_word_set(words_in_post);



            Post p = {content, word_set, label};

            // This automatically creates a p.label key if doesnt exist yet
            label_map[p.label].push_back(p);
            total_training_posts += 1;
        }
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
            words_in_post.push_back(curr_word);
            vocabulary_set.insert(curr_word);
            label_words_map[label].insert(curr_word);

            // Create the set of words
            set<string> word_set = create_word_set(words_in_post);

            Post p = {content, word_set, label};

            // This automatically creates a p.label key if doesnt exist yet
            label_map[p.label].push_back(p);
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