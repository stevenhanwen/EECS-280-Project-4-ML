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
    map<string, vector<Post>> label_map;

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

    int find_num_label_and_word(string word, string label)
    {   
        int total = 0; 
        for (size_t i = 0; i < label_map[label].size(); ++i)
        {   
            auto& post_word_set = label_map[label][i].words;
            auto it = post_word_set.find("word");
            if (it != post_word_set.end())
            {
                total += 1; 
            }
        }

        return total; 
    }


public:
    Classifier()
    {
        total_training_posts = 0;
    }

    Classifier(csvstream &train_csv_in)
    {
        total_training_posts = 0;

        // Rows have key = column name, value = cell datum
        map<string, string> row;
        while (train_csv_in >> row)
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

            // Create the set of words
            set<string> word_set = create_word_set(words_in_post);

            Post p = {content, word_set, label};

            cout << p.label << endl; 

            // This automatically creates a p.label key if doesnt exist yet
            label_map[p.label].push_back(p);
            total_training_posts += 1;
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

    string train_file_name = argv[1];
    // Open file
    try
    {
        csvstream csvin(train_file_name);
        Classifier classifier(csvin);
    }
    catch (const csvstream_exception &e)
    {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}