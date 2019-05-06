#include "Parser.h"
#include <stack>
#include <sstream>
#include <regex>

int token_t::tokenTabs = 0;
Parser::Parser(const Parser & other) : Parser(other.root)
{
}
Parser::Parser(Parser && other)
{
	root = other.root;
	other.root = nullptr;
}
Parser::Parser(const token_t * otherRoot)
{
	(*root) = otherRoot;
}
Parser & Parser::operator=(const Parser & other)
{
	(*root) = other.root;
	return *this;
}
Parser & Parser::operator=(Parser && other)
{
	root = other.root;
	other.root = nullptr;
	return *this;
}
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
	std::regex re(":");
	auto searchStart = data.cbegin() + start;
	auto searchEnd = data.cbegin() + end;
	std::smatch match;
	while (std::regex_search(searchStart, searchEnd, match, re)) {
		size_t pos = match.position() + (searchStart - data.cbegin());
		if (isJSONChar(pos)) {
			if (data[pos + 1] == '{') {
				searchStart = match.suffix().first;
				continue; //already got in scope pass
			}
			int i = 1;
			while (data[pos - ++i] != '"');
			std::string label = data.substr(pos - i + 1, i - 2);
/*			if (!validLabel(label)) {
				searchStart = match.suffix().first;
				continue;
			}*/
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
		}
		searchStart = match.suffix().first;
	}
	return tokens;
}
bool JSONParser::validLabel(const std::string & label)
{
	for (auto c : label)
		if (c != '_' && !isalpha(c)) return false;
	return true;
}
bool JSONParser::isJSONChar(size_t index)
{
	size_t lo = 0, hi = quotes.size() - 1;
	if (index < quotes[lo].first || index > quotes[hi].second) return true;
	while (lo <= hi) {
		size_t mid = (hi + lo) / 2;//lo + (hi - lo) * ((index - quotes[lo].first) / (quotes[hi].second - quotes[lo].first));
		if (index >= quotes[mid].first && index <= quotes[mid].second) return false;
		if (quotes[mid].first > index)
			hi = mid - 1;
		else
			lo = mid + 1;
	}
	return true;

}
void JSONParser::loadStrings(const std::string & data)
{
	for (size_t i = 0; i < data.size(); ++i) {
		if (data[i] == '"') {
			size_t start = i;
			while (data[++i] != '"');
			quotes.emplace_back(start, i);
		}
	}
}
void JSONParser::parse(const std::string & data) {
	loadStrings(data);
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
		bool isc = isJSONChar(position);
		if (matches.str() == "{" && isc) {
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
		else if(isc){
//			printf("Endscope %s\n", parent->name.c_str());
			parent->end = position - 1;
			parent = parent->parent;
		}
//		if (data[position] != '{' && data[position] != '}') printf("mismatch\n");
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

token_t::token_t(const token_t * other) : name(other->name), value(other->value), children(other->children)
{

}

token_t& token_t::operator=(const token_t * other)
{
	name = other->name;
	value = other->value;
	children = other->children;
	return *this;
}

const token_t * token_t::search(const char * label) const
{
	for (auto c : children)
		if (c->name == label) return c;
	return nullptr;
}
