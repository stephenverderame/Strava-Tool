#include "Parser.h"
#include <stack>
#include <sstream>
#include <regex>

int token_t::tokenTabs = 0;
Parser::~Parser()
{
	if (root != nullptr)
		delete root;
}

token_t * Parser::search(const std::string & s, token_t * start, const char * parent) {
	std::stack<token_t*> toSearch;
	toSearch.push(start);
	while (!toSearch.empty()) {
		token_t * n = toSearch.top();
		if (n->name == s) return n;
		else if (parent != nullptr && n->name == parent) {
			return search(s, n, nullptr);
		}
		toSearch.pop();
		for (auto c : n->children)
			toSearch.push(c);

	}
	return nullptr;
}
std::string Parser::search(const std::string & s, const char * parent) {
	token_t * n = search(s, root, parent);
	return n == nullptr ? std::string() : n->value;
}
void Parser::set(const std::string & var, const std::string & val, const char * parent) {
	token_t * n = search(var, root, parent);
	if (n != nullptr) n->value = val;
}
struct scope_t {
	scope_t * parent;
	size_t start, end;
	std::vector<scope_t*> children;
	std::string name;
	~scope_t() {
		for (auto c : children)
			delete c;
	}
};
token_t * JSONParser::parse(const std::string & data, scope_t * scope)
{
	token_t * t = new token_t(scope->name);
	std::vector<token_t*> tokens;
	size_t start = scope->start;
	for (scope_t * sc : scope->children) {
		auto newTokens = parse(data, start, sc->start);
		tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
		tokens.push_back(parse(data, sc));
		start = sc->end;
	}
	auto newTokens = parse(data, start, scope->end);
	tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
	t->children = tokens;
	return t;

}
std::vector<token_t*> JSONParser::parse(const std::string & data, size_t start, size_t end)
{
	std::vector<token_t*> tokens;
/*	std::string s = data.substr(start, end - start);
	std::istringstream iss(s);
	std::string line;
	while (std::getline(iss, line, ',')) {
		size_t pivot = line.find(':');
		if (pivot == std::string::npos) continue;
		std::string label = Util::removeAll(line.substr(0, pivot), '"');
		std::string value = Util::removeAll(line.substr(pivot + 1), '"');
		token_t * n = new token_t(label, value);
		tokens.push_back(n);
	}*/
	std::regex re(":");
	auto searchStart = data.cbegin() + start;
	auto searchEnd = data.cbegin() + end;
	std::smatch match;
	while (std::regex_search(searchStart, searchEnd, match, re)) {
		size_t pos = match.position() + (searchStart - data.cbegin());
		if (data[pos + 1] == '{') {
			searchStart = match.suffix().first;
			continue; //already got in scope pass
		}
		int i = 1;
		while (data[pos - ++i] != '"');
		std::string label = data.substr(pos - i + 1, i - 2);
		i = 1;
		std::string value;
		if (data[pos + 1] == '[') {
			while (data[pos + ++i] != ']' && pos + i <= end);
			value = data.substr(pos + 2, i - 2);
		}
		else {
			while (data[pos + ++i] != ',' && pos + i <= end);
			value = Util::removeAll(data.substr(pos + 1, i - 1), '"');
		}
		tokens.push_back(new token_t(label, value));
		searchStart = match.suffix().first;
	}
	return tokens;
}
void JSONParser::parse(const std::string & data) {
	std::smatch matches;
	scope_t * rootScope = nullptr;
	scope_t * parent = nullptr;
	auto searchStart = data.cbegin();
	std::regex re("\\{|\\}");
	if (data[0] == '[') {
		rootScope = new scope_t{ nullptr, 1, data.size() - 2 };
		rootScope->name = "Root";
		parent = rootScope;
	}
	while (std::regex_search(searchStart, data.cend(), matches, re)) {
		size_t position = matches.position() + (searchStart - data.cbegin());
		if (matches.str() == "{") {
			if (rootScope == nullptr) {
				rootScope = new scope_t{ nullptr, position + 1 };
				rootScope->name = "Root";
				parent = rootScope;
			}
			else {
				scope_t * n = new scope_t{ parent, position + 1 };
//				printf("Scope ");
				if (position > 1 && data[position - 1] == ':') {
					size_t start = position - 2;
					while (data[--start] != '"');
					n->name = data.substr(start + 1, position - 2 - (start + 1));
//					printf("(%s)", n->name.c_str());
				}
				parent->children.push_back(n);
				parent = n;
//				printf("\n");
			}
		}
		else if(matches.str() == "}"){
//			printf("Endscope %s\n", parent->name.c_str());
			parent->end = position - 1;
			parent = parent->parent;
		}
		if (data[position] != '{' && data[position] != '}') printf("mismatch\n");
		searchStart = matches.suffix().first;
	}
//	printf("Scopes init\n");
	if (rootScope == nullptr) {
		fprintf(stderr, "No brackets?\n");
		return;
	}
	this->root = parse(data, rootScope);
	delete rootScope;
}

void JSONParser::addParameter(std::string & name, std::string & value)
{
}

void JSONParser::addParameter(token_t * token)
{
}

std::string JSONParser::format()
{
	return std::string();
}

std::ostream & JSONParser::operator<<(std::ostream & o)
{
	root->operator<<(o);
	return o;
}

std::string Util::removeAll(const std::string & s, char c)
{
	std::stringstream ss;
	for (char let : s)
		if (let != c) ss << let;
	return ss.str();
}

std::string Util::removeAll(const std::string && s, char c)
{
	std::stringstream ss;
	for (char let : s)
		if (let != c) ss << let;
	return ss.str();
}

std::string Util::removeAll(const std::string & s, std::vector<char> chars)
{
	std::stringstream ss;
	for (char let : s)
		if (std::find(chars.begin(), chars.end(), let) == chars.end()) ss << let;
	return ss.str();
}

const token_t * token_t::search(const char * label) const
{
	for (auto c : children)
		if (c->name == label) return c;
	return nullptr;
}
