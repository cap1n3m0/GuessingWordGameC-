#include <iostream>
#include <Windows.h>
#include <list>
#include <map>
#include <vector>
#include <random>
#include <thread>
#include <algorithm>

#define CLR system("cls");	

using namespace std;
using namespace std::chrono_literals; 
using namespace chrono;

class Level {
public:
	int words;
	int time;
	int max_blanks;
	list<string> word_bank;
	virtual void set() = 0;
};

class Level1 : public Level {
public:
	void set() {
		words = 5;
		time = 10;
		max_blanks = 2;
		word_bank = { "overrun", "proximity", "misfit",
		"belligerence", "beret", "bicycle",
		"jacuzzi", "government", "hydraulics"
		};
	}
};

class Level2 : public Level {
public:
	void set() {
		words = 5;	
		time = 8;
		max_blanks = 3;
		word_bank = { "association", "turquoise", "accommodation",
			"remunerate", "liaison", "conscience", "pharaoh",
			"deterrence", "dalmatian", "superstitious", "commission", "clandestine"
		};
	}

};

class Level3 : public Level {
public:
	void set() {
		words = 5;
		time = 8;
		max_blanks = 5;
		word_bank = { "pneumonia", "defalcation", "echelon",
			"gelatinous", "contemptuous", "logorrhea", "discernible",
			"assuage", "bourgeois", "obfuscate", "acquiesce", "andragogy" 
		};
	}
};

// Method declaration
void play();
void set_word();
void ask_word();
void get_length();
void set_blank_count();
void set_console_position(int x, int y);
int getX();
int getY();
void pick_elements();
void print_elements();
void check_entry();
void rw();

Level* l1 = new Level1;
Level* l2 = new Level2;
Level* l3 = new Level3;
static Level* levels[] = { l1, l2, l3 };
static vector<int> blank_letters;
static map<int, char> entry_id;
static int opt = 0;
static int score = 0;
static int level_count = 1;
static int word_l = 0;
static int num_blanks = 1;
static int posX, posY;
static int new_num = 0;
static int time_left = 10;
static char letters[50];
static char rand_letters[50];
static bool is_playing = true;
static bool is_timing = false;
static bool correct = true;
static bool wait = true; 
static string thread_id; 
auto current_level = levels[level_count - 1]; 

int main() {
	l1->set();
	l2->set();
	l3->set();
	play(); 
	return 0;
}

void play() {
	CLR;
	wait = true; 
	entry_id.clear();
	blank_letters.clear();
	correct = true;
	is_playing = true;
	word_l = 0;
	num_blanks = 1;
	new_num = 0;
	current_level = levels[level_count - 1];
	thread score_keeper {
		[&]() {
			int _x, _y; 
			_x = getX();
			_y = getY();
			//set_console_position(0, 0);
			cout << "Level: " << level_count << endl;
			//set_console_position(0, 1);
			cout << "Score: " << score << endl;
			//set_console_position(0, 2);
			cout << endl; 
			is_timing = true; 
		}
	};
	thread game{
	[]() {
			while (is_playing) {
				set_word();
				ask_word();
			}
		}
	};
	score_keeper.join();
	game.join();
	score_keeper.detach(); 
    game.detach(); 
}

int getX() {
	CONSOLE_SCREEN_BUFFER_INFO CONSOLE;
	COORD RESULT;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CONSOLE)) {
		return -1;
	}
	return CONSOLE.dwCursorPosition.X;
}

int getY() {
	CONSOLE_SCREEN_BUFFER_INFO CONSOLE;
	COORD RESULT;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CONSOLE)) {
		return -1;
	}
	return CONSOLE.dwCursorPosition.Y;
}

void set_console_position(int r, int c) {
	COORD screen;
	HANDLE h_out{ GetStdHandle(STD_OUTPUT_HANDLE) };
	screen.X = r;
	screen.Y = c;
	SetConsoleCursorPosition(h_out, screen);
}

void set_word() {
	// Pick random word from word bank
	auto bank = current_level->word_bank;
	unsigned int* rand_num = nullptr;
	bool found = false; 
	int temp = 0;
	for (int i = 0; i < 50; i++) {
		letters[i] = NULL; 
	}
	while(!found) {
		random_device rand;
		mt19937 range(rand());
		uniform_int_distribution<mt19937::result_type> dist(1, bank.size());
		rand_num = new auto(dist(range));
		for (list<string>::iterator it = bank.begin(); it != bank.end(); it++) {
			string word = *it;
			if (temp == *rand_num - 1) {
				for (int j = 0; j < word.length(); j++) {
					letters[j] = word.at(j);
				}
			}
			temp++;
		}
		for (list<string>::iterator it = bank.begin(); it != bank.end(); it++) {
			if (static_cast<string>(letters) == *it) {
				found = true;
				break;
			}
		}
	}
	delete rand_num;
	temp = 0;
}

void ask_word() {
	blank_letters.clear();
	get_length();
	set_blank_count();
	pick_elements();
	print_elements();
	if (time_left > 0) {
		check_entry();
		rw();
	}
	else {
		cout << score;
		cout << "Press any key to restart" << endl;
		cin >> opt;
		score = 0;
		level_count = 1;
		cout << "Restarted" << endl; 
		play(); 
	}
}

void get_length() {
	// letters replaced by blank
	for (auto i : letters) {
		if (i) {
			word_l++;
		}
	}
}

void set_blank_count() {
	random_device rand;
	mt19937 range(rand());
	uniform_int_distribution<mt19937::result_type> dist(1, current_level->max_blanks);
	num_blanks = dist(range);
}

void pick_elements() {
	while (new_num < num_blanks) {
		new_num = 0;
		blank_letters.clear();
		for (int i = 1; i <= num_blanks; i++) {
			random_device rand;
			mt19937 range(rand());
			uniform_int_distribution<mt19937::result_type> dist(1, word_l - 1);
			unique_ptr<int> temp(new int(dist(range)));
			// Checking if element already exists 
			if (find(blank_letters.begin(), blank_letters.end(), *temp) == blank_letters.end()) {
				new_num++;
				blank_letters.push_back(*temp);
			}
		}
	}
	sort(blank_letters.begin(), blank_letters.end());
}

void print_elements() {
	list<char> show_letter;
	list<char> ::iterator i = show_letter.begin();
	bool _push = false;
	time_left = current_level->time;
	is_timing = true;
	show_letter.clear();
	cout << endl;
	for (int i = 0; i < word_l; i++) {
		_push = false;
		for (auto it = blank_letters.begin(); it != blank_letters.end(); it++) {
			if (i + 1 == *it) {
				_push = true;
				show_letter.push_back('_');
			}
			else if (it == blank_letters.end() - 1) {
				if (!_push) {
					show_letter.push_back(letters[i]);
				}
			}
		}
	}
	cout << endl; 
	cout << endl;
	cout << endl;
	cout << "Can you fill in the blank(s)? " << endl;
	for (list<char>::iterator i = show_letter.begin(); i != show_letter.end(); i++) {
		cout << *i << " ";
	}
	thread change_time{
		[&]() {
			while (time_left > 0 && is_timing && wait) {
				this_thread::sleep_for(milliseconds(1000));
				time_left--;
				posX = getX();
				posY = getY();
				set_console_position(0, 2);
				cout << "Timer: " << time_left << endl;
				set_console_position(posX, posY);
			}
			if (time_left <= 0) {
				cout << "Time up!" << endl; 
				cin >> opt; 
				score = 0;
				level_count = 1; 
				is_timing = false; 
				play(); 
			}
		}
	};
	thread fill_thread{
		[&]() {
			char temp;
			cout << endl;
			is_timing = true;
			for (int i = 0; i < blank_letters.size(); i++) {
				cout << "Enter blank " << i << ": " << endl;
				cin >> temp;
				if (isdigit(temp)) {
					wait = false;
					cout << "Invalid choice" << endl;
					cout << "Press any key to restart" << endl;
					score = 0;
					level_count = 1;
					cin >> opt;
					play();
				}
				entry_id.insert(pair<int, char>(blank_letters[i], temp));
				if (i == blank_letters.size() - 1) {
					is_timing = false;
				}
			}
		}
	};
	change_time.join();
	fill_thread.join();
	show_letter.clear();
	if (time_left <= 0) {
		change_time.detach();
		cout << "Try again" << endl;
		cin >> opt;
		play();
	}
}

void check_entry() {
	for (map<int, char> ::iterator it = entry_id.begin(); it != entry_id.end(); it++) {
		int* _temp = new int(it->first);
		if (letters[*_temp - 1] != it->second) {
			correct = false;
		}
		delete _temp;
	}
}

void rw () {
	cout << correct << endl;
	if (correct) {
		cout << "Correct!" << endl;
		score++;
		if (score == current_level->words) {
			level_count++;
			score = 0;
		}
	}
	else {
		cout << "Incorrect!" << endl;
		cout << "Correct word: " << letters << endl;
		score = 0;
		level_count = 1;
	}
	cout << "Press any key to continue" << endl;
	cin >> opt;
	play();
}
