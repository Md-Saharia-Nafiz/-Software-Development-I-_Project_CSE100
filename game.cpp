#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <conio.h>
#include <windows.h>

using namespace std;

// FIRST CODE: Circle Rush Game

class CR_Profile
{
public:
    string name;
    int highScore;

    CR_Profile() : name(""), highScore(0) {}
    CR_Profile(const string &n, int hs) : name(n), highScore(hs) {}

    string historyFile() const
    {
        string s = name;
        for (char &c : s)
        {
            if (c == ' ') c = '_';
            if (c == '/' || c == '\\' || c == '|' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>')
                c = '_';
        }
        return "history_" + s + ".txt";
    }
};

class CR_Ball
{
public:
    int x, y;
    CR_Ball(int _x, int _y) : x(_x), y(_y) {}
};

class CR_Player
{
public:
    int x;
    CR_Player(int startX = 0) : x(startX) {}
    void moveLeft()
    {
        x--;
        if (x < 0) x = 0;
    }
    void moveRight(int width)
    {
        x++;
        if (x > width - 2) x = width - 2;
    }
};

class CR_ProfileManager
{
private:
    string filename;
public:
    vector<CR_Profile> profiles;

    CR_ProfileManager(const string &file) : filename(file) {}

    void createDefaultProfiles()
    {
        profiles.clear();
        profiles.emplace_back("Shadon", 40);
        profiles.emplace_back("Alif", 51);
        profiles.emplace_back("Dhiren", 38);
        profiles.emplace_back("Nahid", 33);
        profiles.emplace_back("Nafiz", 42);
        for (auto &p : profiles)
        {
            ofstream hout(p.historyFile(), ios::app);
            hout.close();
        }
        saveProfiles();
    }

    void loadProfiles()
    {
        profiles.clear();
        ifstream fin(filename);
        if (!fin.is_open())
        {
            createDefaultProfiles();
            return;
        }
        string line;
        while (getline(fin, line))
        {
            if (line.empty()) continue;
            size_t sep = line.find('|');
            if (sep == string::npos) continue;
            string name = line.substr(0, sep);
            int hs = 0;
            try
            {
                hs = stoi(line.substr(sep + 1));
            }
            catch (...)
            {
                hs = 0;
            }
            profiles.emplace_back(name, hs);
            ofstream hout(profiles.back().historyFile(), ios::app);
            hout.close();
        }
        fin.close();
        if (profiles.empty()) createDefaultProfiles();
    }

    void saveProfiles()
    {
        ofstream fout(filename, ios::trunc);
        if (!fout.is_open())
        {
            cerr << "Error opening profiles file.\n";
            return;
        }
        for (const auto &p : profiles) fout << p.name << "|" << p.highScore << "\n";
        fout.close();
    }

    vector<int> loadHistory(const CR_Profile &p)
    {
        vector<int> hist;
        ifstream fin(p.historyFile());
        if (!fin.is_open()) return hist;
        string line;
        while (getline(fin, line))
        {
            if (line.empty()) continue;
            try
            {
                hist.push_back(stoi(line));
            }
            catch (...) {}
        }
        fin.close();
        return hist;
    }

    void appendHistory(const CR_Profile &p, int score)
    {
        ofstream fout(p.historyFile(), ios::app);
        if (!fout.is_open()) return;
        fout << score << "\n";
        fout.close();
    }

    void clearHistory(CR_Profile &p)
    {
        ofstream fout(p.historyFile(), ios::trunc);
        fout.close();
    }
};

class CR_Game
{
private:
    const int WIDTH = 25;
    const int HEIGHT = 20;
    vector<CR_Ball> balls;
    CR_Player player;
    int score;
    int missed;
    int speed;
    int maxMissed;
    bool gameOver;
    CR_Profile &profile;
    CR_ProfileManager &pm;

public:
    CR_Game(CR_Profile &p, CR_ProfileManager &mgr)
        : player(WIDTH / 2), score(0), missed(0), gameOver(false), profile(p), pm(mgr)
    {
        srand((unsigned)time(0));
        speed = 220;
        maxMissed = 50;
    }

    void draw()
    {
        system("cls");
        cout << " Player Name: " << profile.name << "   High Score: " << profile.highScore << "\n";
        cout << "+";
        for (int i = 0; i < WIDTH; i++) cout << "-";
        cout << "+\n";
        for (int i = 0; i < HEIGHT; i++)
        {
            cout << "|";
            for (int j = 0; j < WIDTH; j++)
            {
                bool printed = false;
                if (i == HEIGHT - 1 && (j == player.x || j == player.x + 1))
                {
                    cout << "0";
                    printed = true;
                }
                for (auto &b : balls) if (b.x == j && b.y == i)
                    {
                        cout << "B";
                        printed = true;
                        break;
                    }
                if (!printed) cout << ".";
            }
            cout << "|\n";
        }
        cout << "+";
        for (int i = 0; i < WIDTH; i++) cout << "-";
        cout << "+\n";
        cout << "Score: " << score << "    Missed: " << missed << "/" << maxMissed << endl;
        cout << "\nControls: Right = A/a ; Left = K/k ; Pause = P/p ; Quit = Q/q\n";
    }

    void handleInput()
    {
        while (_kbhit())
        {
            char c = _getch();
            if (c == 'A' || c == 'a') player.moveRight(WIDTH);
            if (c == 'K' || c == 'k') player.moveLeft();
            if (c == 'Q' || c == 'q') gameOver = true;
            if (c == 'P' || c == 'p') pauseScreen();
        }
    }

    void pauseScreen()
    {
        system("cls");
        cout << " PAUSED \nPress R/r to resume or Q/q to quit.\n";
        while (true)
        {
            if (_kbhit())
            {
                char c = _getch();
                if (c == 'R' || c == 'r') break;
                if (c == 'Q' || c == 'q')
                {
                    gameOver = true;
                    break;
                }
            }
            Sleep(100);
        }
    }

    void updateBalls()
    {
        if (rand() % 10 < 2) balls.emplace_back(rand() % WIDTH, 0);
        for (auto &b : balls) b.y++;
        for (int i = 0; i < (int)balls.size(); i++)
        {
            if (balls[i].y == HEIGHT - 1 && (balls[i].x == player.x || balls[i].x == player.x + 1))
            {
                score++;
                Beep(750, 50);
                balls.erase(balls.begin() + i);
                i--;
            }
            else if (balls[i].y >= HEIGHT)
            {
                missed++;
                Beep(400, 80);
                balls.erase(balls.begin() + i);
                i--;
            }
        }
    }

    void run()
    {
        while (!gameOver)
        {
            handleInput();
            updateBalls();
            draw();
            if (missed >= maxMissed) gameOver = true;
            Sleep(speed);
        }
        endGameMenu();
    }

    void endGameMenu()
    {
        system("cls");
        cout << "\n\n\tGAME OVER\n";
        bool beatHigh = (score >= profile.highScore);
        if (beatHigh) cout << endl << "\tYOU WIN!\n";
        else cout << endl << "\tYOU LOSS!\n";
        cout << endl;
        cout << "\tPlayer Name: " << profile.name << "\n";
        cout << endl;
        cout << "\tScore: " << score << "  Missed: " << missed << "\n";

        pm.appendHistory(profile, score);
        if (beatHigh)
        {
            cout << endl << "\n\tYour new High score: " << score << " (old: " << profile.highScore << ")\n";
            profile.highScore = score;
            pm.saveProfiles();
        }
        else
        {
            cout << endl << "\n\tProfile High Score remains: " << profile.highScore << "\n";
        }

        cout << "\n\n1. Go to the Player Menu.\n\n2. Start Over the Game.\n\n3. Quit The Game.\n";
        cout << endl;
        cout << "Please Enter your Choice: ";
        char choice;
        cin >> choice;
        switch (choice)
        {
        case '1':
            return;
        case '2':
            score = 0;
            missed = 0;
            balls.clear();
            gameOver = false;
            run();
            return;
        case '3':
            exit(0);
        default:
            return;
        }
    }
};

void startCircleRushGame()
{
    CR_ProfileManager pm("profiles.txt");
    pm.loadProfiles();

    while (true)
    {
        system("cls");
        cout << "Welcome to Circle Rush Game !\n\n1. Choose The player.\n\n2. Player's Profile & History.\n\n3. Go Back to Main Menu.\n";
        cout << endl;
        cout << "Please Enter Your Choice: ";
        int choice;
        cin >> choice;

        if (choice == 1)
        {
            while (true)
            {
                system("cls");
                cout << "Player's Profile:\n";
                cout << endl;
                for (size_t i = 0; i < pm.profiles.size(); i++)
                    cout << endl << i + 1 << ". " << pm.profiles[i].name << "  HighScore: " << pm.profiles[i].highScore << "\n";
                cout << endl;
                cout << pm.profiles.size() + 1 << ". Go to the Menu\n\nPlease Enter Your Choice: ";
                int pchoice;
                cin >> pchoice;
                if (pchoice >= 1 && pchoice <= (int)pm.profiles.size())
                {
                    CR_Profile &selected = pm.profiles[pchoice - 1];
                    CR_Game game(selected, pm);
                    game.run();
                    pm.saveProfiles();
                }
                else if (pchoice == (int)pm.profiles.size() + 1) break;
            }
        }
        else if (choice == 2)
        {
            while (true)
            {
                system("cls");
                cout << " Player's Profile & History \n";
                cout << endl;
                for (size_t i = 0; i < pm.profiles.size(); i++)
                {
                    cout << endl;
                    cout << i + 1 << ". " << pm.profiles[i].name << "  HighScore: " << pm.profiles[i].highScore << "\nHistory: ";
                    vector<int> h = pm.loadHistory(pm.profiles[i]);
                    if (h.empty()) cout << "No records";
                    else for (auto sc : h) cout << sc << " ";
                    cout << "\n";
                }
                cout << endl;
                cout << pm.profiles.size() + 1  << ". Go to the Menu\n\nSelect profile number to clear history or " << pm.profiles.size() + 1 << " to go menu: ";
                char inp = _getch();
                int idx = inp - '1';
                if (idx >= 0 && idx < (int)pm.profiles.size())
                {
                    pm.clearHistory(pm.profiles[idx]);
                    cout << "\nHistory cleared! Press any key to continue...";
                    _getch();
                }
                else if (idx == (int)pm.profiles.size()) break;
            }
        }
        else if (choice == 3) return;
    }
}

// SECOND CODE: RPS Challenge Game

class RPS_Player
{
private:
    string name;
    int win;
    int loss;
    string lastHistory;
public:
    RPS_Player() {}
    RPS_Player(string n, int w, int l, string h = "No Match Played Yet!")
    {
        name = n;
        win = w;
        loss = l;
        lastHistory = h;
    }

    string getName()
    {
        return name;
    }
    void winGame()
    {
        win++;
        lastHistory = "WON the last match!";
    }
    void loseGame()
    {
        loss++;
        lastHistory = "LOST the last match!";
    }
    void clearHistory()
    {
        lastHistory = "History Cleared!";
    }

    void showProfile()
    {
        cout << name << " -> WIN=" << win << ", LOSS=" << loss << "\nLast Game: " << lastHistory << endl;
    }
    void showScoreOnly()
    {
        cout << name << " -> WIN=" << win << ", LOSS=" << loss << endl;
    }

    string toFileString()
    {
        return name + " " + to_string(win) + " " + to_string(loss) + " " + lastHistory;
    }
    void fromFileString(string line)
    {
        int firstSpace = line.find(' ');
        int secondSpace = line.find(' ', firstSpace + 1);
        int thirdSpace = line.find(' ', secondSpace + 1);
        name = line.substr(0, firstSpace);
        win = stoi(line.substr(firstSpace + 1, secondSpace - firstSpace - 1));
        loss = stoi(line.substr(secondSpace + 1, thirdSpace - secondSpace - 1));
        lastHistory = line.substr(thirdSpace + 1);
    }
};

class RPS_Game
{
private:
    vector<RPS_Player> players;
public:
    RPS_Game()
    {
        loadFromFile();
        if (players.empty())
        {
            players.push_back(RPS_Player("Shadon", 8, 4));
            players.push_back(RPS_Player("Alif", 11, 6));
            players.push_back(RPS_Player("Dhiren", 5, 9));
            players.push_back(RPS_Player("Nahid", 7, 10));
            players.push_back(RPS_Player("Nafiz", 13, 6));
            saveToFile();
        }
    }

    void saveToFile()
    {
        ofstream fout("rps_players.txt");
        for (auto &p : players) fout << p.toFileString() << endl;
        fout.close();
    }

    void loadFromFile()
    {
        ifstream fin("rps_players.txt");
        if (!fin) return;
        players.clear();
        string line;
        while (getline(fin, line))
        {
            RPS_Player p;
            p.fromFileString(line);
            players.push_back(p);
        }
        fin.close();
    }

    void mainMenu()
    {
        while (true)
        {
            system("CLS");
            cout << "Welcome to RPS Challenge Game !\n\n1. Choose the Player.\n\n2. Player's Profile & History.\n\n3. Go Back to Main Menu.\n\nPlease Enter Your Choice: ";
            int op;
            cin >> op;
            if (op == 1) choosePlayer();
            else if (op == 2) showProfiles();
            else if (op == 3) return;
        }
    }

    void choosePlayer()
    {
        system("CLS");
        cout << "\n Player's Profile \n\n";
        for (int i = 0; i < players.size(); i++)
        {
            cout << i + 1 << ". ";
            players[i].showScoreOnly();
            cout << endl;
        }
        cout << players.size() + 1 << ". Go to the Menu\n\nPlease Enter Your Choice: ";
        int p;
        cin >> p;
        if (p == players.size() + 1) return;
        if (p < 1 || p > players.size())
        {
            cout << "Invalid Player!\n";
            return;
        }
        startGame(p - 1);
    }

    void showProfiles()
    {
        system("CLS");
        cout << "\n Player's Profile & History \n\n";
        for (int i = 0; i < players.size(); i++)
        {
            cout << i + 1 << ". ";
            players[i].showProfile();
            cout << endl;
        }
        cout << "Clear history? (C/c) or any key to go menu: ";
        char ch;
        cin >> ch;
        if (ch == 'c' || ch == 'C') clearHistory();
    }

    void clearHistory()
    {
        cout << "Enter Player Number: ";
        int n;
        cin >> n;
        if (n >= 1 && n <= players.size())
        {
            players[n - 1].clearHistory();
            saveToFile();
            cout << "History Cleared!\n";
        }
        else cout << "Invalid Player Number!\n";
    }

    int getComputerChoice()
    {
        return rand() % 3 + 1;
    }
    string choiceName(int n)
    {
        if (n == 1) return "Rock";
        if (n == 2) return "Paper";
        return "Scissors";
    }

    void startGame(int index)
    {
        srand(time(0));
        while (true)
        {
            system("CLS");
            cout << "Rock = 1, Paper = 2, Scissors = 3\n\nPause = P/p ; Resume = R/r\n\nEnter your Choice: ";
            char x;
            cin >> x;
            if (x == 'P' || x == 'p')
            {
                cout << endl << "Paused! Press R/r...\n";
                while (true)
                {
                    cin >> x;
                    if (x == 'R' || x == 'r') break;
                }
                continue;
            }
            int playerChoice = x - '0';
            if (playerChoice < 1 || playerChoice > 3)
            {
                cout << "Invalid Choice!\n";
                continue;
            }
            int comp = getComputerChoice();
            cout << "\n\nYou chose: " << choiceName(playerChoice) << "\n\nComputer chose: " << choiceName(comp) << endl;
            if (playerChoice == comp) cout << "\nDRAW!\n";
            else if ((playerChoice == 1 && comp == 3) || (playerChoice == 2 && comp == 1) || (playerChoice == 3 && comp == 2))
            {
                players[index].winGame();
                cout << "\nYOU WIN!\n";
            }
            else
            {
                players[index].loseGame();
                cout << "\nYOU LOSS!\n";
            }
            saveToFile();
            cout << "\n\n1. Go to the Main Menu.\n\n2. Start Over the Game.\n\n3. Quit The Game. \n\nPlease Enter your Choice: ";
            int op;
            cin >> op;
            if (op == 1) return;
            if (op == 3) exit(0);
        }
    }
};

// MAIN MENU

int main()
{
    while (true)
    {
        system("cls");
        cout << "Welcome! Choose which game you want to play:\n\n1. Circle Rush Game\n\n2. RPS Challenge Game\n\n3. Exit\n\nPlease enter your preferred number: ";
        int choice;
        cin >> choice;
        if (choice == 1) startCircleRushGame();
        else if (choice == 2)
        {
            RPS_Game rps;
            rps.mainMenu();
        }
        else if (choice == 3) break;
        else
        {
            cout << "Invalid! Press any key...\n";
            _getch();
        }
    }
    return 0;
}
