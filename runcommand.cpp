#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

class Trie {
private:
	struct trie {
		trie *firstChild, *nxtBrother;
		char value;
		void* data;
	};
	trie* rt;
	void clear(trie* p, void (*freeValue)(void*)) {
		trie* j = p->firstChild, *r;
		while(j != 0x0) {
			clear(j, freeValue);
			r = j->nxtBrother;
			if(freeValue != 0x0 && j->data != 0x0) freeValue(j->data); 
			free(j);
			j = r;
		}
	}
public:
	Trie() {
		rt = (trie*)memset(malloc(sizeof(trie)), 0, sizeof(trie));
	}
	~Trie() {
		clear(rt, 0x0);
		free(rt);
	}
	void clear(void (*freeValue)(void*) =0x0) { return clear(rt, freeValue); }
	class iterator {
	private:
		trie* M_current;
	public:
		iterator(trie* p =0x0): M_current(p) {}
		~iterator() {}
		iterator(const iterator& it): M_current(it.M_current) {}
		void*& operator*() { return M_current->data; }
		bool operator == (const iterator& it) const { return M_current == it.M_current; }
		bool operator != (const iterator& it) const { return M_current != it.M_current; }
	};
	iterator end() const { return iterator(0x0); }
	iterator find(const std::string& str) const { return find(str.c_str(), str.length()); }
	iterator find(const char* str, unsigned len) const {
		trie* p = rt;
		for(unsigned i = 0; i < len; ++i) {
			trie* j = p->firstChild;
			while(j != 0x0) if(j->value == str[i]) break; else j = j->nxtBrother;
			p = j;
			if(p == 0x0) break;
		}
		if(p == 0x0 || p->value == 0x0) return end();
		return iterator(p);
	}
	iterator insert(const char* str, unsigned len, void* value, void (*freeValue)(void*) =0x0) {
		trie* p = rt;
		for(unsigned i = 0; i < len; ++i) {
			trie* j = p->firstChild;
			while(j != 0x0) if(j->value == str[i]) break; else j = j->nxtBrother;
			if(j == 0x0) {
				j = (trie*)memset(malloc(sizeof(trie)), 0, sizeof(trie));
				j->nxtBrother = p->firstChild;
				p->firstChild = j;
				j->value = str[i];
			}
			p = j;
		}
		if(p->data != 0x0) if(freeValue != 0x0) freeValue(p->data);
		p->data = value;
		return iterator(p);
	}
};

static const int MAJOR_VERSION = 5;
static const int MINOR_VERSION = 0;
const char defaultFilename[16] = "runcommand.txt";
Trie variables, labels;
bool boolOption(const std::string&);

const char* usage = "\
# Pass paths of text files where there are option to be done by args. If no file is given, the program will read 'runcommand.txt' by default.\n\
# In the text, one line for one command.\n\
# For comment, start this line with '#'\n\
# If you want to stop when some command goes error, start this line with '!'\n\
# If you want to skip some command when the previous one succeeds, start this line with '|'\n\
# If you want to skip some command when the previous one fails, start this line with '&'\n\
# If you want to set a variable, start this line with '@'. The name of the variable should closely follow '@'. After the name there should be a space(note that not '\\t'), from the first non-space character to the end of line is the value of this variable, including spaces.\n\
# To use a variable, mention its name with '@' in a command line.\n\
# There are a few built-in variables. They are switch options. They work when they are set and their value is not \"false\" (all small letters) \n\
# Following is a simple example.\n\
@QUIT_WHEN_DONE\n\
# when \"@QUIT_WHEN_DONE\" is set, you needn't to press enter to continue when a file ends.\n\
@QUIT_WHEN_ERROR\n\
# when \"@QUIT_WHEN_ERROR\" is set, the rest commands after a failed one with the mark '!' are skipped.\n\
@HELP\n\
# when \"@HELP\" is set, add this help message to the end of file.\n\
command1\n\
# command1 will be run.\n\
! command2\n\
# command2 will be run, regardless of whether command1 succeed. If it failed, the program will pause to ask whether this file should continue.\n\
# If @QUIT_WHEN_ERROR is set, the rest commands will be skipped once command2 failed, without asking.\n\
|! command3\n\
# command3 is skipped if command2 succeeds. If command3 is not skipped and it failed, the program will pause as it did on command2.\n\
command4\n\
# command4 will be run.\n\
& command5\n\
# command5 is skipped if command4 fails.\n\
@var1 a simple variable\n\
# define a variable named \"var1\", with its value \"a simple variable\"\n\
cd @var1 .txt\n\
# use variable @var1. this command will be pre-processed into \"cd a simple variable.txt\". Note that a space is missed.\
# variable @@ will be pre-processed into @. A single @ will also be pre-processed into @, nothing changed.\
+Label with Space\n\
# mark the beginning of a label.\n\
command6\n\
command7\n\
# command6 and command7 will be run, with the value of var1 \"a simple variable\"\n\
-Label with Space\n\
# mark the end of a label.\n\
@var1 myVar\n\
# reset var1\n\
:Label with Space\n\
# run commands in the label. Here command6 and command7 will be run, with the value of var1 \"myVar\".\n\
|:Label with Space\n\
# all commands in the label will be run only when the previous command(here the second reach of command7) failed.\n\
";

char* content;
unsigned len;
int result;

void initLabels(const char*, unsigned, unsigned);
int work(const char*, unsigned, unsigned, std::string* =0x0);
std::string CHRAT("@");
template <class T>
void freeType(void* p) {
	delete (T*)p;
}

typedef void(*freeFunc)(void*);
freeFunc freeVar = freeType<std::string>, freeLabel = freeType<std::pair<unsigned, unsigned> >;

int main(int argc, const char** argv) {
	printf("runcommand version: %d.%d\n", MAJOR_VERSION, MINOR_VERSION);
	if(argc == 1) argv[argc++] = defaultFilename;
	for(int i = 1; i < argc; ++i) {
		if(strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--help", 6) == 0 || strncmp(argv[i], "-H", 2) == 0) {
			printf("\n# Usage of %s\n# Version: %d.%d\n%s\n", argv[0], MAJOR_VERSION, MINOR_VERSION, usage);
			continue;
		}
		FILE* infile = fopen(argv[i], "r");
		if(infile == 0x0) {
			printf("Can't open %s!\n", argv[i]);
		}else {
			variables.clear(freeType<std::string>);
			labels.clear(freeType<std::pair<unsigned, unsigned> >);
			variables.insert("@", 1, &CHRAT, freeVar);
			variables.insert("", 0, &CHRAT, freeLabel);
			result = 0;
			printf("Reading from %s\n", argv[i]);
			fseek(infile, 0, SEEK_END);
			len = ftell(infile);
			fseek(infile, 0, SEEK_SET);
			content = (char*)malloc(sizeof(char) * (len + 10));
			content[len = fread(content, 1, len, infile)] = 10;
			initLabels(content, 0, len);

			result = work(content, 0, len);

			free(content);
			if(result & 1) {
				FILE* outfile = fopen(argv[i], "a");
				fprintf(outfile, "\n# Usage of %s\n# Version: %d.%d\n%s\n", argv[0], MAJOR_VERSION, MINOR_VERSION, usage);
				fclose(outfile);
			}
			if((result & 2) == 0) {
				printf("Press Enter to Continue/Exit...");
				fflush(stdin);
				getchar();
			}
		}
	}
	return 0;
}

bool boolOption(const std::string& str) {
	Trie::iterator it = variables.find(str);
	if(it == variables.end() || *(std::string*)*it == "false") return false;
	return true;
}

void initLabels(const char* content, unsigned origin, unsigned len) {
	unsigned st, ed;
	len += (st = ed = origin);
	Trie::iterator it;
	std::string labelName;
	while((ed = st) < len) {
		while(content[++ed] != 10);
		if(content[st] == '+' || content[st] == '-') {
			labelName.assign(content + st + 1, ed - st - 1);
			if((it = labels.find(labelName)) == labels.end()) {
				it = labels.insert(labelName.c_str(), labelName.length(), new std::pair<unsigned, unsigned>(-1u, -1u), freeLabel);
			}
			std::pair<unsigned, unsigned>& pr = *(std::pair<unsigned, unsigned>*)*it;
			if(content[st] == '+') for(pr.first = ed; content[++pr.first] == 10; );
			else pr.second = st - 1;
		}
		st = ed;
		while(content[++st] == 10);
	}
}

int work(const char* content, unsigned origin, unsigned len, std::string* labelName) {
	unsigned st, ed;
	len += (st = ed = origin); --ed;
	while((st = ++ed) < len) {
		if(content[ed] == 10) continue;
		while(content[++ed] != 10);
		if(content[st] == '#' || content[st] == '+' || content[st] == '-') continue;
		if(content[st] == '@') {
			unsigned i = st;
			while(++i < ed) if(content[i] == 0x20) break;
			if(i == ed) variables.insert(content + st + 1, i - st - 1, new std::string(), freeVar);
			else variables.insert(content + st + 1, i - st - 1, new std::string(content + i + 1, ed - i - 1), freeVar);
			continue;
		}
		if(content[st] == '|' && result == 0) continue;
		if(content[st] == '&' && result != 0) continue;
		if(content[st] == '|' || content[st] == '&') ++st;
		if(content[st] == ':') {
			std::pair<unsigned, unsigned>* pr = (std::pair<unsigned, unsigned>*)*labels.find(content + st + 1, ed - st - 1);
			if(pr == 0x0) {
				fwrite("[Error] Label \"", 1, 16, stdout);
				fwrite(content + st + 1, 1, ed - st - 1, stdout);
				fwrite("\" Not Find!\n", 1, 13, stdout);
				fflush(stdout);
			} else if(pr->first == -1u || pr->second == -1u) {
				fwrite("[Error] Label\"", 1, 16, stdout);
				fwrite(content + st + 1, 1, ed - st - 1, stdout);
				fwrite("\" Uncompleted!\n", 1, 16, stdout);
				fflush(stdout);
			} else work(content, pr->first, pr->second - pr->first, new std::string(content + st + 1, ed - st - 1));
			continue;
		}
		bool askBreak = false;
		if(content[st] == '!') {
			askBreak = true;
			++st;
		}
		std::string command;
		while(st < ed) {
			if(content[st] == '@') {
				unsigned j = st;
				while(j < ed && content[j] != ' ' && content[j] != '\n') ++j;
				Trie::iterator it = variables.find(content + st + 1, j - st - 1);
				if(it != variables.end()) command.append(*(std::string*)*it);
				st = j;
			} else command.push_back(content[st]);
			++st;
		}
		if(labelName != 0x0) printf(":%s", labelName->c_str());
		printf("> %s\n", command.c_str());
		if((result = system(command.c_str())) != 0) {
			printf("[Error] %s\nWIth Code: %d\n", command.c_str(), result);
			if(askBreak) {
				if(boolOption("QUIT_WHEN_ERROR")) break;
				printf("Continue? [enter 'n' to break]:");
				fflush(stdin);
				if(getchar() == 'n') {
					variables.insert("QUIT_WHEN_DONE", 14, new std::string, freeVar);
					break;
				}
			}
		}
	}
	if(labelName == 0x0) printf("--Done--\n");
	else free(labelName);
	return boolOption("HELP") | (boolOption("QUIT_WHEN_DONE") << 1);
}