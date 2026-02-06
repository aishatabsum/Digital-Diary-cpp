//------------*******MINI DIGITAL DIARY*******--------
//A console friendly application for saving multi-line notes, to-do tasks and contacts
//comes with a user authentication system to ensure security in personal diary.
 
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
//to map each user to his key
#include <unordered_map>
//to add colors to console
#include <windows.h>

using namespace std;

// --------------------- Configuration ---------------------
const string INDENT = "                      "; // soft left padding
const int MAX_NOTES = 140;
const int MAX_TASKS = 140;
const int MAX_CONTACTS = 140;
const string USERS_FILE = "users.txt";
const string NOTES_FILE = "notes.txt";
const string TASKS_FILE = "tasks.txt";
const string CONTACTS_FILE = "contacts.txt";

// --------------------- ANSI Color Macros ------------------
#define RESET "\033[0m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BG_BLUE_WHITE "\033[44;37m"
#define BOLD "\033[1m"
#define GREEN "\033[32m"

// --------------------- Data Structures -------------------
struct Note {
    string datetime;   // timestamp when note added
    string content;    // multi-line content (contains '\n')
};

struct Task {
    string text;      // single-line task description
    bool done;        // completed or not
    string datetime;  // when added (dd/mm/yyyy hh:mm)
};

struct Contact {
    string name;
    string phone;
    string datetime; // when added
};

// ------- In-memory storage ----------
Note notes[MAX_NOTES];
int noteCount = 0;

Task tasks[MAX_TASKS];
int taskCount = 0;

Contact contacts[MAX_CONTACTS];
int contactCount = 0;

// ----------- Utilities --------------
// Remove trailing CR for Windows formatted files
string trimCR(const string& s);
// Return current date/time as dd/mm/yyyy hh:mm
string getDateTime();
//Enable ANSI escape sequences on Windows console 
void enableAnsi();
// Safe read whole line after using >> or similar
void safeIgnoreNewline() ;

// ---------------------- Users ----------------------------

// Load users
void loadUsers(unordered_map<string,string>& users);
// Save users 
void saveUsers(const unordered_map<string,string>& users);

// -------------------- Notes I/O -------------------------
void rewriteNotesFile();
void loadNotes();
void appendNoteToFile(const Note& n);
// -------------------- Tasks I/O -------------------------
void rewriteTasksFile();
void loadTasks();
void appendTaskToFile(const Task& t);
// -------------------- Contacts I/O ----------------------
void rewriteContactsFile();
void loadContacts();
void appendContactToFile(const Contact& c);
// -------------------- UI Helpers ------------------------
void showTitle();
// -------------------- Notes UI --------------------------
void addNote() ;
void viewNotes();
void deleteSingleNote();
void deleteAllNotes() ;
// -------------------- Tasks UI --------------------------
void addTask() ;
void viewTasks();
void toggleOrDeleteTask();
//-------------------- Contacts UI -----------------------
void addContact();
void viewContacts();
void deleteSingleContact();
void deleteAllContacts();
// ---------------------- Menus ---------------------------
void notesMenu();
void tasksMenu();
void contactsMenu();
// ----------------------- Main ---------------------------
int main() {
    // Enable ANSI colors on Windows 
    enableAnsi();

    // Load users
    unordered_map<string,string> users;
    loadUsers(users);

    // Login sequence (plain text keys)
    cout << BG_BLUE_WHITE << BOLD << "\n   MINI DIGITAL DIARY LOGIN   \n" << RESET << endl;
    string name, key;
    cout << "Enter your name: ";
    cin >> name;
    if (users.find(name) == users.end()) {
        cout << "New user! Set secret key: ";
        cin >> key;
        users[name] = key;
        saveUsers(users);
    // CLEAR DATA FILES FOR NEW USER
    ofstream nf(NOTES_FILE); nf.close();
    ofstream tf(TASKS_FILE); tf.close();
    ofstream cf(CONTACTS_FILE); cf.close();

    cout << GREEN << "Registered successfully!" << RESET << "\n";
}

        else {
        cout << "Enter your secret key: ";
        cin >> key;
        if (users[name] != key) {
            cout << RED << "Incorrect key. Access denied." << RESET << "\n";
            return 0;
        }
        cout << GREEN << "Welcome back, " << name << "!" << RESET << "\n";
    }

    // Load saved data
    loadNotes();
    loadTasks();
    loadContacts();
    showTitle();

    // Main menu loop
    while (true) {
        cout << "\n" << INDENT << BG_BLUE_WHITE << " MAIN MENU " << RESET << "\n";
        cout << INDENT << "1. Notes\n" << INDENT << "2. Tasks\n" << INDENT << "3. Contacts\n" << INDENT << "0. Exit\n";
        cout << INDENT << "Choose: ";
        int m; cin >> m;
        if (m == 1) notesMenu();
        else if (m == 2) tasksMenu();
        else if (m == 3) contactsMenu();
        else if (m == 0) { cout << "\n" << INDENT << CYAN << "Exiting. Goodbye!" << RESET << "\n"; break; }
        else cout << INDENT << RED << "Invalid choice." << RESET << "\n";
    }

    return 0;
}
// ---------------------- Utilities ------------------------
// Remove trailing CR for Windows formatted files
string trimCR(const string& s) {
    if (!s.empty() && s.back() == '\r') return s.substr(0, s.size()-1);
    return s;
}

//Return real time date/time in
// the string format (dd/mm/yyyy hh:mm)
string getDateTime() {
    time_t now = time(0); 
    tm* ltm = localtime(&now);
    char buf[20];
    //string format time
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", ltm);
    return string(buf);
}

//enabling console colours
//to enable windows support for
//Unix-based ANSI colours
void enableAnsi() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode)) return;
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

// Safe read whole line after using >> or similar
void safeIgnoreNewline() {
    if (cin.peek() == '\n') cin.ignore(); 
	//peek looks at next character
}

// ---------------------- Users ----------------------------
// Load users (username -> key) from USERS_FILE
void loadUsers(unordered_map<string,string>& users) {
    users.clear();
    ifstream fin(USERS_FILE);
    if (!fin) return;
    string name, key;
    while (fin >> name >> key) {
        users[name] = key;
    }
    fin.close();
}

// Save users map back to USERS_FILE (overwrites)
//one-one mapping
void saveUsers(const unordered_map<string,string>& users) {
    ofstream fout(USERS_FILE);
    for (const auto& p : users) fout << p.first << " " << p.second << "\n";
    fout.close();
}

//-------- Notes format ---------
// File block format (notes.txt):
// DATE: dd/mm/yyyy hh:mm
// CONTENT:
// <line1>
// <line2>
// ~
// ---

//overwriting notes
void rewriteNotesFile() {
    ofstream fout(NOTES_FILE);
    for (int i = 0; i < noteCount; ++i) {
        fout << "DATE: " << notes[i].datetime << "\n";
        fout << "CONTENT:\n";
        fout << notes[i].content;
        fout << "~\n";
        fout << "---\n";
    }
    fout.close();
}

//loading notes
void loadNotes() {
    ifstream fin(NOTES_FILE);
    if (!fin) { noteCount = 0; return; }
    noteCount = 0;
    string line;
    while (getline(fin, line) && noteCount < MAX_NOTES) {
        line = trimCR(line);
        if (line.rfind("DATE:", 0) == 0) {
            Note n;
            n.datetime = line.substr(5);
			//from 5 index to the end of this line.
            if (!n.datetime.empty() && n.datetime[0] == ' ') 
			n.datetime = n.datetime.substr(1);
            // expect CONTENT: next
            if (!getline(fin, line)) break; // CONTENT:
            n.content.clear();
            while (getline(fin, line)) {
                line = trimCR(line);
                if (line == "~") break; // end of content
                n.content += line + "\n";
            }
            // consume separator (---) if present
            getline(fin, line);
            notes[noteCount++] = n;
        }
    }
    fin.close();
}

//adding notes
void appendNoteToFile(const Note& n) {
    ofstream fout(NOTES_FILE, ios::app);
    fout << "DATE: " << n.datetime << "\n";
    fout << "CONTENT:\n";
    fout << n.content;
    fout << "~\n";
    fout << "---\n";
    fout.close();
}

// -------------------- Tasks I/O -------------------------
// File single-line format (tasks.txt):
// [0] task text | dd/mm/yyyy hh:mm

//overwriting tasks after editing
void rewriteTasksFile() {
    ofstream fout(TASKS_FILE);
    for (int i = 0; i < taskCount; ++i) {
        fout << "[" << (tasks[i].done ? '1' : '0') << "] " << tasks[i].text << " | " << tasks[i].datetime << "\n";
    }
    fout.close();
}

//loading saved tasks
void loadTasks() {
    ifstream fin(TASKS_FILE);
    if (!fin) { taskCount = 0; return; }
    taskCount = 0;
    string line;
    while (getline(fin, line) && taskCount < MAX_TASKS) {
        line = trimCR(line);
        if (line.size() >= 3 && line[0] == '[' && (line[1] == '0' || line[1] == '1') && line[2] == ']') {
            Task t;
            t.done = (line[1] == '1');
            // find start of text after "] "
            size_t posTextStart = (line.size() > 3 && line[3] == ' ') ? 4 : 3;
            string rest = (posTextStart < line.size()) ? line.substr(posTextStart) : string();
            // look for " | " delimiter to separate text and datetime
            size_t delim = rest.rfind(" | ");
            if (delim != string::npos) {
                t.text = rest.substr(0, delim);
                t.datetime = rest.substr(delim + 3);
            } else {
                // fallback: no datetime present in file (old format) -> keep text, set datetime empty
                t.text = rest;
                t.datetime = ""; 
				// will show empty if not present
            }
            tasks[taskCount++] = t;
        }
    }
    fin.close();
}

//adding tasks 
void appendTaskToFile(const Task& t) {
    ofstream fout(TASKS_FILE, ios::app);
    fout << "[" << (t.done ? '1' : '0') << "] " << t.text << " | " << t.datetime << "\n";
    fout.close();
}

// -------------------- Contacts I/O ----------------------
// File block format (contacts.txt):
// NAME: name
// PHONE: phone
// DATE: dd/mm/yyyy hh:mm
// ---

//overwriting contacts after editing
void rewriteContactsFile() {
    ofstream fout(CONTACTS_FILE);
    for (int i = 0; i < contactCount; ++i) {
        fout << "NAME: " << contacts[i].name << "\n";
        fout << "PHONE: " << contacts[i].phone << "\n";
        fout << "DATE: " << contacts[i].datetime << "\n";
        fout << "---\n";
    }
    fout.close();
}
//loading contacts
void loadContacts() {
    ifstream fin(CONTACTS_FILE);
    if (!fin) { contactCount = 0; return; }
    contactCount = 0;
    string line;
    while (getline(fin, line) && contactCount < MAX_CONTACTS) {
        line = trimCR(line);
        if (line.rfind("NAME:", 0) == 0) {
            Contact c;
            c.name = line.substr(5);
            if (!c.name.empty() && c.name[0] == ' ') c.name = c.name.substr(1);

            // PHONE:
            if (getline(fin, line)) {
                line = trimCR(line);
                if (line.rfind("PHONE:", 0) == 0) {
                    c.phone = line.substr(6);
                    if (!c.phone.empty() && c.phone[0] == ' ') c.phone = c.phone.substr(1);
                } else {
                    c.phone = "";
                }
            }

            // DATE:
            if (getline(fin, line)) {
                line = trimCR(line);
                if (line.rfind("DATE:", 0) == 0) {
                    c.datetime = line.substr(5);
                    if (!c.datetime.empty() && c.datetime[0] == ' ') c.datetime = c.datetime.substr(1);
                } else {
                    c.datetime = "";
                }
            } else {
                c.datetime = "";
            }
            // consume separator line (---) if present
            getline(fin, line);
            contacts[contactCount++] = c;
        }
    }
    fin.close();
}
//adding contacts
void appendContactToFile(const Contact& c) {
    ofstream fout(CONTACTS_FILE, ios::app);
    fout << "NAME: " << c.name << "\n";
    fout << "PHONE: " << c.phone << "\n";
    fout << "DATE: " << c.datetime << "\n";
    fout << "---\n";
    fout.close();
}

// ---------- UI Helpers -----------
//title function
void showTitle() {
    cout << "\n" << INDENT << BG_BLUE_WHITE << BOLD << "=== MINI DIGITAL DIARY ===" << RESET << "\n";
}

//--------- Notes UI ----------
//adding notes
void addNote() {
    if (noteCount >= MAX_NOTES) { cout << INDENT << RED << "Note storage full." << RESET << "\n"; return; }
    safeIgnoreNewline();
    Note n;
    n.datetime = getDateTime();
    n.content.clear(); 
    cout << INDENT << WHITE << BOLD <<
	 "Enter note (multi-line). End with a single line containing only ~" << RESET << "\n";
    string line;
    while (true) {
        getline(cin, line);
        if (trimCR(line) == "~") break;
        n.content += line + "\n";
    }
    notes[noteCount++] = n;
    appendNoteToFile(n);
    cout << INDENT << CYAN << "Note saved at " << n.datetime << RESET << "\n";
}
//view notes function
void viewNotes() {
    if (noteCount == 0) { cout << INDENT << YELLOW << "No notes." << RESET << "\n"; return; }
    cout << INDENT << WHITE << BOLD << "---- Notes (" << noteCount << ") ----" << RESET << "\n";
    for (int i = 0; i < noteCount; ++i) {
        cout << INDENT << BG_BLUE_WHITE << " Note #" << (i+1) << "  Date: " << notes[i].datetime << RESET << "\n";
        cout << INDENT << "----------------------\n";
        string c = notes[i].content;
        size_t pos = 0;
        while (pos < c.size()) {
            size_t next = c.find('\n', pos);
            string ln = (next == string::npos ? c.substr(pos) : c.substr(pos, next - pos));
            cout << INDENT << "  " << ln << "\n";
            if (next == string::npos) break;
            pos = next + 1;
        }
        cout << INDENT << "----------------------\n\n";
    }
}
//delete single note function
void deleteSingleNote() {
    if (noteCount == 0) { cout << INDENT << YELLOW << "No notes to delete." << RESET << "\n"; return; }
    viewNotes();
    cout << INDENT << YELLOW << "Enter note number to delete (0 cancel): " << RESET;
    int k; cin >> k;
    if (k <= 0 || k > noteCount) { cout << INDENT << YELLOW << "Cancelled." << RESET << "\n"; return; }
    for (int i = k - 1; i < noteCount - 1; ++i) notes[i] = notes[i + 1];
    noteCount--;
    rewriteNotesFile();
    cout << INDENT << CYAN << "Deleted note #" << k << RESET << "\n";
}

//delete all notes functions
void deleteAllNotes() {
    if (noteCount == 0) { cout << INDENT << YELLOW << "No notes to delete." << RESET << "\n"; return; }
    cout << INDENT << RED << "Are you sure you want to DELETE ALL notes? (y/N): " << RESET;
    char ch; cin >> ch;
    if (ch == 'y' || ch == 'Y') {
        noteCount = 0;
        rewriteNotesFile(); // will overwrite with empty
        cout << INDENT << CYAN << "All notes deleted." << RESET << "\n";
    } else {
        cout << INDENT << YELLOW << "Cancelled." << RESET << "\n";
    }
}

// ---------- Tasks UI -----------
//add task function
void addTask() {
    if (taskCount >= MAX_TASKS) { cout << INDENT << RED << "Task storage full." << RESET << "\n"; return; }
    safeIgnoreNewline();
    Task t;
    cout << INDENT << YELLOW << "Enter task description: " << RESET;
    getline(cin, t.text);
    t.done = false;
    t.datetime = getDateTime(); // saved at creation
    tasks[taskCount++] = t;
    appendTaskToFile(t);
    cout << INDENT << CYAN << "Task added at " << t.datetime << RESET << "\n";
}
//view task function
void viewTasks() {
    if (taskCount == 0) { cout << INDENT << YELLOW << "No tasks." << RESET << "\n"; return; }
    cout << INDENT << YELLOW << "---- Tasks ----" << RESET << "\n";
    for (int i = 0; i < taskCount; ++i) {
        cout << INDENT << setw(3) << (i+1) << ". [" << (tasks[i].done ? 'x' : ' ') << "] "
             << tasks[i].text << " (Added: " << (tasks[i].datetime.empty() ? "unknown" : tasks[i].datetime) << ")\n";
    }
}
//toggle or delete function
void toggleOrDeleteTask() {
    if (taskCount == 0) { cout << INDENT << YELLOW << "No tasks." << RESET << "\n"; return; }
    viewTasks();
    cout << INDENT << YELLOW << "Enter task number (0 cancel): " << RESET;
    int k; cin >> k;
    if (k <= 0 || k > taskCount) { cout << INDENT << YELLOW << "Cancelled." << RESET << "\n"; return; }
    cout << INDENT << YELLOW << " 1.Toggle \n" << INDENT << " 2.Delete \n" << INDENT << " Enter choice:  " << RESET;
    int c; cin >> c;
    if (c == 1) {
        tasks[k-1].done = !tasks[k-1].done;
        rewriteTasksFile();
        cout << INDENT << CYAN << "Toggled." << RESET << "\n";
    } else if (c == 2) {
        for (int i = k - 1; i < taskCount - 1; ++i) tasks[i] = tasks[i + 1];
        taskCount--;
        rewriteTasksFile();
        cout << INDENT << CYAN << "Deleted." << RESET << "\n";
    } else {
        cout << INDENT << RED << "Invalid option." << RESET << "\n";
    }
}

// -------------------- Contacts UI -----------------------
//add contact function
void addContact() {
    if (contactCount >= MAX_CONTACTS) { cout << INDENT << RED << "Contacts full." << RESET << "\n"; return; }
    safeIgnoreNewline();
    Contact c;
    cout << INDENT << CYAN << "Enter name: " << RESET; getline(cin, c.name);
    cout << INDENT << CYAN << "Enter phone: " << RESET; getline(cin, c.phone);
    c.datetime = getDateTime();
    contacts[contactCount++] = c;
    appendContactToFile(c);
    cout << INDENT << CYAN << "Contact saved at " << c.datetime << RESET << "\n";
}
//view contacts function
void viewContacts() {
    if (contactCount == 0) { cout << INDENT << YELLOW << "No contacts." << RESET << "\n"; return; }
    cout << INDENT << CYAN << "---- Contacts (" << contactCount << ") ----" << RESET << "\n";
    cout << INDENT; cout.setf(ios::left);
    cout << setw(5) << "No" << setw(30) << "Name" << setw(18) << "Phone" << setw(20) << "Added" << RESET << "\n";
    cout << INDENT << string(73, '-') << "\n";
    for (int i = 0; i < contactCount; ++i) {
        cout << INDENT << setw(5) << (i+1)
             << setw(30) << (contacts[i].name.size() > 28 ? contacts[i].name.substr(0,28) : contacts[i].name)
             << setw(18) << contacts[i].phone << setw(20) << (contacts[i].datetime.empty() ? "unknown" : contacts[i].datetime) << "\n";
    }
    cout.unsetf(ios::left);
}
//delete single contact function
void deleteSingleContact() {
    if (contactCount == 0) { cout << INDENT << YELLOW << "No contacts to delete." << RESET << "\n"; return; }
    viewContacts();
    cout << INDENT << YELLOW << "Enter contact number to delete (0 cancel): " << RESET;
    int k; cin >> k;
    if (k <= 0 || k > contactCount) { cout << INDENT << YELLOW << "Cancelled." << RESET << "\n"; return; }
    for (int i = k - 1; i < contactCount - 1; ++i) contacts[i] = contacts[i + 1];
    contactCount--;
    rewriteContactsFile();
    cout << INDENT << CYAN << "Deleted contact #" << k << RESET << "\n";
}
//delete all contact function
void deleteAllContacts() {
    if (contactCount == 0) { cout << INDENT << YELLOW << "No contacts to delete." << RESET << "\n"; return; }
    cout << INDENT << RED << "Are you sure you want to DELETE ALL contacts? (y/N): " << RESET;
    char ch; cin >> ch;
    if (ch == 'y' || ch == 'Y') {
        contactCount = 0;
        rewriteContactsFile(); // will overwrite with empty
        cout << INDENT << CYAN << "All contacts deleted." << RESET << "\n";
    } else {
        cout << INDENT << YELLOW << "Cancelled." << RESET << "\n";
    }
}

// ---------------------- Menus ---------------------------

void notesMenu() {
    while (true) {
        cout << "\n" << INDENT << WHITE << " -- NOTES -- " << RESET << "\n";
        cout << INDENT << "1. Add Note\n" << INDENT << "2. View Notes\n"
             << INDENT << "3. Delete Single Note\n" << INDENT << "4. Delete All Notes\n"
             << INDENT << "0. Back\n";
        cout << INDENT << "Choose: ";
        int ch; cin >> ch;
        if (ch == 1) addNote();
        else if (ch == 2) viewNotes();
        else if (ch == 3) deleteSingleNote();
        else if (ch == 4) deleteAllNotes();
        else if (ch == 0) break;
        else cout << INDENT << RED << "Invalid." << RESET << "\n";
    }
}

void tasksMenu() {
    while (true) {
        cout << "\n" << INDENT << YELLOW << " -- TASKS -- " << RESET << "\n";
        cout << INDENT << "1. Add Task\n" << INDENT << "2. View Tasks\n"
             << INDENT << "3. Toggle/Delete Task\n" << INDENT << "0. Back\n";
        cout << INDENT << "Choose: ";
        int ch; cin >> ch;
        if (ch == 1) addTask();
        else if (ch == 2) viewTasks();
        else if (ch == 3) toggleOrDeleteTask();
        else if (ch == 0) break;
        else cout << INDENT << RED << "Invalid." << RESET << "\n";
    }
}

void contactsMenu() {
    while (true) {
        cout << "\n" << INDENT << CYAN << " -- CONTACTS -- " << RESET << "\n";
        cout << INDENT << "1. Add Contact\n" << INDENT << "2. View Contacts\n"
             << INDENT << "3. Delete Single Contact\n" << INDENT << "4. Delete All Contacts\n"
             << INDENT << "0. Back\n";
        cout << INDENT << "Choose: ";
        int ch; cin >> ch;
        if (ch == 1) addContact();
        else if (ch == 2) viewContacts();
        else if (ch == 3) deleteSingleContact();
        else if (ch == 4) deleteAllContacts();
        else if (ch == 0) break;
        else cout << INDENT << RED << "Invalid." << RESET << "\n";
    }
}
