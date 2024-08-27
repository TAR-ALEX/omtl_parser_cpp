// BSD 3-Clause License

// Copyright (c) 2022, Alex Tarasov
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

size_t ParseTreeBuilder::findMatchingBracket(std::vector<Token>& tokens, size_t i) {
    for (; i < tokens.size(); i++) {
        if (tokens[i].getRaw() == "[") break;
    }
    i++;
    for (;;) {
        if (i >= tokens.size()) {
            return tokens.size();
        } else if (tokens[i].getRaw() == "[") {
            i = findMatchingBracket(tokens, i) + 1;
        } else if (tokens[i].getRaw() == "]") {
            return i;
        } else {
            i++;
        }
    }
}

//TODO: delete is deprecated
bool ParseTreeBuilder::isTuple(std::vector<Token>& tokens, size_t i) {
    for (; i < tokens.size(); i++) {
        if (tokens[i].getRaw() == "[") break;
    }
    i++;
    for (;;) {
        if (i >= tokens.size()) {
            return false;
        } else if (tokens[i].getRaw() == "]") {
            return false;
        } else if (tokens[i].getRaw() == ":") {
            return true;
        } else if (tokens[i].getRaw() == ",") {
            return true;
        } else if (tokens[i].getRaw() == "[") {
            i = findMatchingBracket(tokens, i) + 1;
        } else {
            i++;
        }
    }
}

Element ParseTreeBuilder::parseStatement(std::vector<Token>& tokens, size_t& i) {
    Element result;
    result.statement = std::deque<std::pair<std::string, Element>>();
    if (tokens.size() > i) result.location = tokens[i].location;
    for (;;) {
        //if (i >= tokens.size()) { throw std::runtime_error("statement did not end: " + result.location); } // error
        if (i >= tokens.size()) { break; }    // done
        if (tokens[i].getRaw() == ",") break; // done
        if (tokens[i].getRaw() == "]") break; // done
        if (tokens[i].getRaw() == ":") {
            throw std::runtime_error("statement did not expect a colon at: " + tokens[i].location);
        }
        if (tokens[i].getRaw() == "[") {
            Element elem = parseTuple(tokens, i);
            result.statement->push_back({std::to_string(result.statement->size()), elem});
        } else {
            Element elem;
            elem.value = tokens[i];
            elem.location = tokens[i].location;
            result.statement->push_back({std::to_string(result.statement->size()), elem});
            i++;
        }
    }
    return result;
}

/*
 parses a tuple with the parentheses example:

 ```
 [a: 1, b: 2, c: 3, d: [e: 4, f: 5 + 4],]
 ```
 */
Element ParseTreeBuilder::parseTuple(std::vector<Token>& tokens, size_t& i, bool isRoot) {
    Element result;
    result.tuple = std::deque<std::pair<std::string, Element>>();
    if (isRoot) {
        result.location = tokens[i].location;
    } else {
        for (; i < tokens.size(); i++) {
            if (tokens[i].getRaw() == "[") {
                result.location = tokens[i].location;
                break;
            }
        }
        i++; // advance inside
    }
    for (;;) {
        if (isRoot) {
            if (i >= tokens.size()) { break; }
            if (tokens[i].getRaw() == "]") {
                throw std::runtime_error("tuple too many closing braces: " + result.location);
                break;
            }
        } else {
            if (i >= tokens.size()) { throw std::runtime_error("tuple did not end: " + result.location); } // error
            if (tokens[i].getRaw() == "]") {
                i++;
                break;
            }
        }
        if (tokens[i].getRaw() == ":") {
            throw std::runtime_error("unexpected colon in tuple at: " + tokens[i].location);
        } // error
        if (tokens[i].getRaw() == ",") {
            i++;
            continue;
        } // skip over
        std::string name = "";
        Element statement;
        if (i + 1 < tokens.size() && tokens[i + 1].getRaw() == ":") {
            if (!tokens[i].isName()) // TODO: support strings, only names are supported for now
                throw std::runtime_error("unexpected tag in tuple at: " + tokens[i].location);
            if (i + 2 >= tokens.size() ||
                tokens[i + 2].getRaw() == ",") { // we have a named item with an empty statement under it.
                //throw std::runtime_error("empty statement in tuple at: " + tokens[i].location);
                //skip over it instead
                i += 3;
                continue;
            }
            name = tokens[i].getRaw();
            i += 2;
            statement = parseStatement(tokens, i);
        } else {
            name = result.tuple->size();
            statement = parseStatement(tokens, i);
        }
        result.tuple->push_back(std::pair<std::string, Element>(name, statement));
    }

    return result;
}

Element ParseTreeBuilder::buildParseTree(std::vector<Token> tokens) {
    size_t startIndex = 0;
    return parseTuple(tokens, startIndex, true);
}

Element::Element() {}

Element Element::Tuple(std::deque<std::pair<std::string, Element>> t) {
    Element e;
    e.tuple = t;
    return e;
}

Element Element::Tuple(std::deque<Element> s) {
    Element e;
    e.tuple = std::deque<std::pair<std::string, Element>>();
    for (size_t i = 0; i < s.size(); i++) { e.tuple->push_back({std::to_string(i), s[i]}); }
    return e;
}

Element Element::Statement(std::deque<Element> s) {
    Element e;
    e.statement = std::deque<std::pair<std::string, Element>>();
    for (size_t i = 0; i < s.size(); i++) { e.statement->push_back({std::to_string(i), s[i]}); }
    return e;
}

Element::Element(Token v) { value = v; }

Element::Element(const Element& e) {
    tuple = e.tuple;
    statement = e.statement;
    value = e.value;
}

std::string Element::getDiagnosticString() {
    using namespace std;
    using namespace estd::string_util;
    if (this->tuple != nullptr) {
        string result = "";
        size_t idx = -1;
        for (std::pair<string, Element> namedStatement : this->tuple) {
            idx++;
            if (namedStatement.first != "") {
                result += namedStatement.first;
                result += ": ";
            } else {
                // result += to_string(idx);
                // result += ": ";
            }

            result += namedStatement.second.getDiagnosticString();
            result += ", \n";
        }
        if (result == "") result = "[]";
        else
            result = "[\n" + indent(result, "   ") + "]";
        return result;
    } else if (this->statement != nullptr) {
        string result = "";
        bool notFirst = false;
        for (auto [label, e] : this->statement) {
            if (notFirst) { result += " "; }
            notFirst = true;
            if (e.isStatement()) {
                result += "[" + e.getDiagnosticString() + "]";
            } else {
                result += e.getDiagnosticString();
            }
        }
        return result;
    } else if (this->value != nullptr) {
        return this->value->getRaw();
    }
    return "unknown type";
}

Element Element::operator[](std::string name) {
    Element& e = *this;

    if (e.tuple != nullptr) {
        for (size_t i = 0; i < e.tuple->size(); i++) {
            if (e.tuple[i].first == name || name == std::to_string(i)) { return e.tuple[i].second; }
        }
    } else if (e.statement != nullptr) {
        for (size_t i = 0; i < e.statement->size(); i++) {
            if (e.statement[i].first == name || name == std::to_string(i)) { return e.tuple[i].second; }
        }
    }
    throw std::runtime_error("tuple does not contain element "+name+ ": " + e.location);
}

Element Element::operator[](size_t id) {
    Element& e = *this;
    if (e.tuple != nullptr) {
        if (id >= e.tuple->size()) throw std::runtime_error("tuple does not contain element "+std::to_string(id)+ ": " + e.location);
        return e.tuple[id].second;
    } else if (e.statement != nullptr) {
        if (id >= e.statement->size()) throw std::runtime_error("statement does not contain element "+std::to_string(id)+ ": " + e.location);
        return e.statement[id].second;
    }
    throw std::runtime_error("Element is not a tuple or statement: " + e.location);
}

size_t Element::size() {
    Element& e = *this;
    if (e.tuple != nullptr) {
        return e.tuple->size();
    } else if (e.statement != nullptr) {
        return e.statement->size();
    }
    return 0;
}

bool Element::onlyContains(std::set<std::string> names) {
    // TODO: make this work for statement (you can just use size currently)
    if (tuple == nullptr) return false;
    for (size_t i = 0; i < this->tuple->size(); i++) {
        if (names.find(this->tuple[i].first) == names.end()) { return false; }
    }
    return true;
}

bool Element::contains(std::string name) {
    if (tuple != nullptr) {
        for (size_t i = 0; i < this->tuple->size(); i++) {
            if (this->tuple[i].first == name || name == std::to_string(i)) { return true; }
        }
    } else if (this->statement != nullptr) {
        try {
            if (std::stoull(name, nullptr, 10) <= this->statement->size()) return true;
        } catch (...) {}
    }
    return false;
}

bool Element::contains(size_t id) {
    if (this->tuple != nullptr) {
        if (id >= this->tuple->size()) return false;
        return true;
    } else if (this->statement != nullptr) {
        if (id >= this->statement->size()) return false;
        return true;
    }
    return false;
}

Element Element::popFront() {
    if (tuple != nullptr && tuple->size() != 0) {
        Element t = tuple->front().second;
        tuple->pop_front();
        return t;
    }
    if (statement != nullptr && statement->size() != 0) {
        Element t = statement->front().second;
        statement->pop_front();
        return t;
    }
    throw std::runtime_error("No elements to pop");
}
void Element::pushFront(Element e) { pushFront("", e); }
void Element::pushFront(std::string n, Element e) {
    if (tuple != nullptr) {
        if (n == "") {
            tuple->push_front({std::to_string(tuple->size()), e});
        } else {
            if (n[0] >= '0' && n[0] <= '9') { throw std::runtime_error("cannot add a number label to a tuple"); }
            tuple->push_front({n, e});
        }
    } else if (statement != nullptr) {
        if (n != "") { throw std::runtime_error("cannot add a label to a statement"); }
        statement->push_front({std::to_string(statement->size()), e});
    } else {
        throw std::runtime_error("cannot push element");
    }
}
Element Element::popBack() {
    if (tuple != nullptr && tuple->size() != 0) {
        Element t = tuple->back().second;
        tuple->pop_back();
        return t;
    }
    if (statement != nullptr && statement->size() != 0) {
        Element t = statement->back().second;
        statement->pop_back();
        return t;
    }
    throw std::runtime_error("No elements to pop");
}

Element Element::front() {
    if (tuple != nullptr && tuple->size() != 0) {
        Element t = tuple->front().second;
        return t;
    }
    if (statement != nullptr && statement->size() != 0) {
        Element t = statement->front().second;
        return t;
    }
    throw std::runtime_error("No elements to pop");
}

Element Element::back() {
    if (tuple != nullptr && tuple->size() != 0) {
        Element t = tuple->back().second;
        return t;
    }
    if (statement != nullptr && statement->size() != 0) {
        Element t = statement->back().second;
        return t;
    }
    throw std::runtime_error("No elements to pop");
}

void Element::pushBack(Element e) { pushBack("", e); }
void Element::pushBack(std::string n, Element e) {
    if (tuple != nullptr) {
        if (n == "") {
            tuple->push_back({std::to_string(tuple->size()), e});
        } else {
            if (n[0] >= '0' && n[0] <= '9') { throw std::runtime_error("cannot add a number label to a tuple"); }
            tuple->push_back({n, e});
        }
    } else if (statement != nullptr) {
        if (n != "") { throw std::runtime_error("cannot add a label to a statement"); }
        statement->push_back({std::to_string(statement->size()), e});
    } else {
        throw std::runtime_error("cannot push element");
    }
}

void Element::popBack(size_t n) { // TODO: make a faster implementation
    for (size_t i = 0; i < n; i++) popBack();
}

void Element::popFront(size_t n) { // TODO: make a faster implementation
    for (size_t i = 0; i < n; i++) popFront();
}

Element Element::slice(size_t left, size_t right) {
    Element copy = *this;
    if (tuple == nullptr && statement == nullptr) throw std::runtime_error("No elements to slice");
    if (right < size()) copy.popBack(size() - right);
    copy.popFront(left);
    return copy;
}


bool Element::isTuple() { return this->tuple != nullptr; }
bool Element::isEmptyTuple() { return this->tuple != nullptr && this->tuple->size() == 0; }
bool Element::isStatement() { return this->statement != nullptr; }
bool Element::isToken() {
    Element& e = getSingleElement();
    return e.value != nullptr;
}

bool Element::isString() { return isToken() && getToken().isString(); }
bool Element::isComment() { return isToken() && getToken().isComment(); }
bool Element::isName() { return isToken() && getToken().isName(); }
bool Element::isNumber() { return isToken() && getToken().isNumber(); }
bool Element::isValue() { return isToken() && getToken().isValue(); }
bool Element::isRaw() { return isToken(); }

Token Element::getToken() {
    Element& e = getSingleElement();
    return e.value.value();
}
std::string Element::getString() { return getToken().getString(); }
std::string Element::getEscapedString() { return getToken().getEscapedString(); }
std::string Element::getComment() { return getToken().getComment(); }
std::string Element::getName() { return getToken().getName(); }
estd::BigDec Element::getNumber() { return getToken().getNumber(); }
std::string Element::getValue() { return getToken().getValue(); }
std::string Element::getRaw() { return getToken().getRaw(); }
Element& Element::getSingleElement() {
    if (statement != nullptr && statement->size() == 1) { return statement[0].second; }
    return *this;
}