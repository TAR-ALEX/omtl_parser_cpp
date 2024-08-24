// BSD 3-Clause License

// Copyright (c) 2024, Alex Tarasov
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

#include <estd/UnitTest.h>
#include <omtl/ParseTree.hpp>

using namespace std;
using namespace estd;
using namespace omtl;
using namespace estd::string_util;

UnitTests tests;

omtl::Element testStrToTree(std::string contents) {
    stringstream fakefile(contents);

    Tokenizer t;
    auto tokens = t.tokenize(fakefile, "test.txt");
    ParseTreeBuilder ptb;
    return ptb.buildParseTree(tokens);
}

void literalsTests() {
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(11)");
        return pt[0][0]->getNumber() == 11;
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(0b11)");
        return pt[0][0]->getNumber() == 3;
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(11.500)");
        return pt[0][0]->getNumber() == 11.50;
    });
    tests.testLambda([]() {
        const std::string kLargeNumber = "7750231786409182364578901326401623401236478102635071238641920384761902384";
        auto pt = testStrToTree(kLargeNumber);
        return pt[0][0]->getNumber() == kLargeNumber;
    });
    tests.testLambda([]() {
        const std::string kLargeNumber =
            "7750231786409182364578901326401623401236478102635071238641920384761902384.0000022384716239784";
        auto pt = testStrToTree(kLargeNumber);
        return pt[0][0]->getNumber() == kLargeNumber;
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"("hello world")");
        return pt[0][0]->getString() == "hello world";
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(hello world)");
        return pt[0][0]->getName() == "hello" && "world" == pt[0][1]->getName();
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(hello: world)");
        return pt["hello"][0]->getName() == "world";
    });
    tests.testLambda([]() {
        auto pt = testStrToTree(R"(hello: "world",)");
        return pt["hello"][0]->getString() == "world";
    });
}

void tupleStructureTests1() {
    string s = R""""(
        [
            one: two three,
            four: [5, ["six\n"]],
            7,8,["nine"]
        ],
        "ten",
    )"""";
    auto pt = testStrToTree(s);

    tests.testBool(pt[0][0]["one"][0]->getName() == "two");
    tests.testBool(pt[0][0]["one"][1]->getName() == "three");
    tests.testBool(pt[0][0]["four"][0][0][0]->getNumber() == 5);
    tests.testBool(pt[0][0]["four"][0][1][0][0][0]->getEscapedString() == "six\\n");
    tests.testBool(pt[0][0][2][0]->getNumber() == "7");
    tests.testBool(pt[0][0][3][0]->getNumber() == "8");
    tests.testBool(pt[0][0][4][0][0][0]->getString() == "nine");
    tests.testBool(pt[1][0]->getString() == "ten");
}

void tupleStructureTests2() {
    string s = R""""(
        "zero",
        "one",
        two: "two",
        three: "three",
        four: [[]],
    )"""";
    auto pt = testStrToTree(s);

    tests.testBool(pt[0][0]->getString() == "zero");
    tests.testBool(pt[1][0]->getString() == "one");
    tests.testBool(pt[2][0]->getString() == "two");
    tests.testBool(pt[3][0]->getString() == "three");

    tests.testBool(pt["two"][0]->getString() == "two");
    tests.testBool(pt["three"][0]->getString() == "three");

    tests.testBool(pt["0"][0]->getString() == "zero");
    tests.testBool(pt["1"][0]->getString() == "one");
    tests.testBool(pt["2"][0]->getString() == "two");
    tests.testBool(pt["3"][0]->getString() == "three");

    tests.testBool(pt["four"][0][0][0]->isEmptyTuple());
}

void testFailsTuple() {
    tests.testBlock({
        bool passed = false;
        try {
            string s = R""""(
                "zero",
                "one"
                "two": "two",
                three: "three",
            )"""";
            auto pt = testStrToTree(s);
        } catch (std::runtime_error& err) {
            passed = estd::string_util::contains(
                err.what(), "statement did not expect a colon at: (file: test.txt line: 4 column: 22)"
            );
            if (!passed) std::cerr << err.what() << std::endl;
        }
        return passed;
    });

    tests.testBlock({
        bool passed = false;
        try {
            string s = R""""(
                "zero",
                "one",
                "two": "two",
                three: "three",
            )"""";
            auto pt = testStrToTree(s);
            std::cerr << pt.getDiagnosticString() << std::endl;
        } catch (std::runtime_error& err) {
            passed = estd::string_util::contains(
                err.what(), "unexpected tag in tuple at: (file: test.txt line: 4 column: 17)"
            );
            if (!passed) std::cerr << err.what() << std::endl;
        }
        return passed;
    });

    tests.testBlock({
        bool passed = false;
        try {
            string s = R""""(
                "zero",
                "one",
                2: "two",
                three: "three",
            )"""";
            auto pt = testStrToTree(s);
            std::cerr << pt.getDiagnosticString() << std::endl;
        } catch (std::runtime_error& err) {
            passed = estd::string_util::contains(
                err.what(), "unexpected tag in tuple at: (file: test.txt line: 4 column: 17)"
            );
            if (!passed) std::cerr << err.what() << std::endl;
        }
        return passed;
    });

    tests.testBlock({
        bool passed = false;
        try {
            string s = R""""(
                [
                    one,
                    [,
                    two,
                ]
            )"""";
            auto pt = testStrToTree(s);
            std::cerr << pt.getDiagnosticString() << std::endl;
        } catch (std::runtime_error& err) {
            passed = estd::string_util::contains(err.what(), "tuple did not end: (file: test.txt line: 2 column: 17)");
            if (!passed) std::cerr << err.what() << std::endl;
        }
        return passed;
    });

    tests.testBlock({
        bool passed = false;
        try {
            string s = R""""(
                [
                    one,
                    [,
                    two,]
                ]]
            )"""";
            auto pt = testStrToTree(s);
            std::cerr << pt.getDiagnosticString() << std::endl;
        } catch (std::runtime_error& err) {
            passed = estd::string_util::contains(
                err.what(), "tuple too many closing braces: (file: test.txt line: 2 column: 17)"
            );
            if (!passed) std::cerr << err.what() << std::endl;
        }

        return passed;
    });
}


void testIndexing() {
    tests.testBlock({
        string s = R""""(
            [
                ,
                ,
                20
            ]
        )"""";
        auto pt = testStrToTree(s);
        return pt[0][0][0][0]->getNumber() == "20";
    });
    tests.testBlock({
        string s = R""""(
            [
                , (ignored, as no statement under it)
                twenty: ,(ignored, as no statement under it)
                20 (first actual statement in the tuple)
            ]
        )"""";
        auto pt = testStrToTree(s);
        return pt[0][0][0][0]->getNumber() == "20";
    });
    tests.testBlock({
        string s = R""""(
            [
                one,
                [,
                two,]
            ]
        )"""";
        auto pt = testStrToTree(s);
        return pt[0][0][1][0][0][0]->getName() == "two";
    });
    tests.testBlock({
        string s = R""""(
            [
                one,
                [[],
                two,]
            ]
        )"""";
        auto pt = testStrToTree(s);
        return pt[0][0][1][0][1][0]->getName() == "two";
    });
}

int main() {
    literalsTests();
    tupleStructureTests1();
    tupleStructureTests2();
    testFailsTuple();
    testIndexing();
    std::cout << tests.getStats() << std::endl;
}