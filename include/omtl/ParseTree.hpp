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

// OMTL - Object Message Tuple List

#pragma once

#include <deque>
#include <estd/ostream_proxy.hpp>
#include <estd/ptr.hpp>
#include <estd/string_util.h>
#include <iostream>
#include <map>
#include <memory>
#include <omtl/ParseTree.hpp>
#include <omtl/Tokenizer.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace omtl {
    class Element {
    private:
        friend class ParseTreeBuilder;
        estd::clone_ptr<std::deque<std::pair<std::string, Element>>> tuple;
        estd::clone_ptr<std::deque<std::pair<std::string, Element>>> statement;
        estd::clone_ptr<Token> value;

        inline Element& getSingleElement();

    public:
        inline static Element Tuple(std::deque<std::pair<std::string, Element>> in);
        inline static Element Statement(std::deque<Element> in = {});
        inline static Element Tuple(std::deque<Element> in = {}); // a function that is a "constructor"
        inline Element();
        inline Element(Token v);
        inline Element(const Element& e);
        inline ~Element() {}

        std::string location = "";

        inline std::string getDiagnosticString();


        inline decltype(auto) begin() {
            if (tuple) {
                return tuple.begin();
            } else {
                return statement.begin();
            }
        }
        inline decltype(auto) begin() const {
            if (tuple) {
                return tuple.begin();
            } else {
                return statement.begin();
            }
        }
        inline decltype(auto) end() {
            if (tuple) {
                return tuple.end();
            } else {
                return statement.end();
            }
        }
        inline decltype(auto) end() const {
            if (tuple) {
                return tuple.end();
            } else {
                return statement.end();
            }
        }

        inline size_t size();
        inline bool onlyContains(std::set<std::string> names);
        inline bool contains(std::string name);
        inline bool contains(size_t id);
        inline Element operator[](std::string name);
        inline Element operator[](size_t id);

        inline Element slice(size_t left, size_t right = SIZE_MAX);
        inline Element front();
        inline Element back();
        inline Element popFront();
        inline Element popBack();
        inline void popFront(size_t n);
        inline void popBack(size_t n);
        inline void pushFront(Element e);
        inline void pushBack(Element e);
        inline void pushFront(std::string n, Element e);
        inline void pushBack(std::string n, Element e);

        inline bool isTuple();
        inline bool isEmptyTuple();
        inline bool isStatement();
        inline bool isToken();

        inline bool isString();
        inline bool isComment();
        inline bool isName();
        inline bool isNumber();
        inline bool isValue();
        inline bool isRaw();

        inline Token getToken();
        inline std::string getString();
        inline std::string getEscapedString();
        inline std::string getComment();
        inline std::string getName();
        inline estd::BigDec getNumber();
        inline std::string getValue();
        inline std::string getRaw();
    };

    class ParseTreeBuilder {
    private:
        inline size_t findMatchingBracket(std::vector<Token>& tokens, size_t i);
        inline bool isTuple(std::vector<Token>& tokens, size_t i);
        inline Element parseStatement(std::vector<Token>& tokens, size_t& i);
        inline Element parseTuple(std::vector<Token>& tokens, size_t& i, bool isRoot = false);

    public:
        inline Element buildParseTree(std::vector<Token> vector);
    };

#include <omtl/ParseTree.ipp>
} // namespace omtl