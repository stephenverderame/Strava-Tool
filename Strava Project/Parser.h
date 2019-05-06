#pragma once
#include <string>
#include <vector>
struct token_t {
	std::string name, value;
	std::vector<token_t*> children;

	inline token_t * addChild(std::string & name, std::string & value) {
//		children.emplace_back(name, value);
		return this;
	}
	inline token_t * addChild(const char * name, const char * value) {
//		children.emplace_back(name, value);
		return this;
	}
	token_t(std::string & name, std::string & value) : name(name), value(value){}
	token_t(std::string & name) : name(name) {};
	token_t(const char * name, const char * value) : name(name), value(value){}
	token_t(const token_t * other);
	token_t& operator=(const token_t * other);
	~token_t() {
		for (auto c : children)
			delete c;
	}
	std::ostream& operator<<(std::ostream & o) const {
		for (int i = 0; i < tokenTabs; ++i)
			o << "\t";
		o << name << ": " << value << "\n";
		++tokenTabs;
		for (token_t * c : children) {
			c->operator<<(o);
		}
		--tokenTabs;
		return o;
	}
	const token_t * search(const char * label) const;
private:
	static int tokenTabs;
};
class Parser
{
protected:
	std::string data;
	token_t * root;
protected:
	token_t * search(const std::string & s, token_t * start, const char * parent = nullptr);
public:
	Parser(const std::string & data) : root(nullptr), data(data) {};
	Parser() : root(nullptr) {};
	Parser(const Parser & other);
	Parser(Parser && other);
	Parser(const token_t * otherRoot);
	Parser& operator=(const Parser & other);
	Parser& operator=(Parser && other);
	~Parser();
	virtual void parse(const std::string & data) = 0;
	virtual void addParameter(std::string & name, std::string & value) = 0;
	virtual void addParameter(token_t * token) = 0;
	virtual std::string format() = 0;
	virtual std::ostream& operator<<(std::ostream& o) = 0;
	std::string search(const std::string & s, const char * parent = nullptr);
	void set(const std::string & var, const std::string & val, const char * parent = nullptr);
	/**
	* Returns root node of parse tree
	* Memory is still managed by parser class. Pointer can't exceed life of Parser class
	*/
	const token_t * rootNode() const { return root; }
};
struct scope_t;
class JSONParser : public Parser {
private:
	std::vector<std::pair<size_t, size_t>> quotes;
private:
	token_t * parse(const std::string & data, scope_t * scope);
	std::vector<token_t*> parse(const std::string & data, size_t start, size_t end);
	/**
	* Determines if a label contains non alphabetical characters
	* @return true, if label contains only alphabetical characters and/or underscores
	* Intended usage supersceded by @see isJSONChar()
	*/
	bool validLabel(const std::string & label);
	/**
	* Determines if the character at the given index is within a string
	* @return, true if the character is not enclosed in a string
	*/
	bool isJSONChar(size_t index);
	/**
	* Finds the range of all JSON data strings
	* Output stored in quotes variable
	*/
	void loadStrings(const std::string & data);
public:
	/**
	* Preconditions: all labels contain letters or _ only
	* 			     data contains json only, enclosed in [] or a root node {}
	*				 no newlines or spaces (spaces in labels or values are ok)
	*/
	void parse(const std::string & data);
	void addParameter(std::string & name, std::string & value);
	void addParameter(token_t * token);
	std::string format();
	std::ostream& operator<<(std::ostream& o);
};
class Util {
public:
	static std::string removeAll(const std::string & s, char c);
	static std::string removeAll(const std::string && s, char c);
	static std::string removeAll(const std::string & s, std::vector<char> chars);
};

