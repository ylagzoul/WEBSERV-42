#include "request.hpp"

Request::Request() : status(0), getEndHeaders(false), getEndFile(false), fdFile(-1){};

std::string tolowerStr(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::tolower(str[i]);
    return (str);
}



// int main()
// {
//     int length_of_blanks;
//     cin >> length_of_blanks; cin.ignore();
//     string full_quote;
//     getline(cin, full_quote);
//     int r = 0;
//     int count;
//     string words;
//     vector<string> joi;
//     int j = 0;
//     for (int i = 0; i < length_of_blanks; i++)
//     {
//         if (full_quote.at(i) == ' ')
//         {
//             count++;
//             joi.push_back(words);
//             words.erase(0, words.length());
//             j = 0;
//         }
//         words[j] = full_quote.at(i);
//         j++;
//         r++;
//     }

//     // Write an answer using cout. DON'T FORGET THE "<< endl"
//     // To debug: cerr << "Debug messages..." << endl;
//     for (int i = 0; i < joi.size(); i++)
//     {
//         if (i == 0)
//             cout << joi[i];
//         else
//             cout << " " << joi[i];
//     }
//     cout << endl;
//     cout << count << " words" << endl;
//     cout << r << " characters long" << endl;

// }