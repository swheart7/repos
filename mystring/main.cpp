// main.cpp - MyString 테스트 및 사용 예제
#include "MyString.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

int main() {
    // 생성
    MyString empty;
    assert(empty.empty() && empty.size() == 0 && empty.c_str()[0] == '\0');
    MyString hello("Hello");
    MyString stars(3, '*');
    assert(hello.size() == 5 && stars == "***");

    // 복사 / 이동
    MyString copy = hello;
    copy[0] = 'J';
    assert(hello == "Hello" && copy == "Jello");
    MyString moved = std::move(copy);
    assert(moved == "Jello" && copy.empty());
    copy = "reused";                         // 이동된 객체 재사용
    assert(copy == "reused");
    moved = moved;                           // 자기 대입
    assert(moved == "Jello");

    // 연결
    MyString s = hello + ", " + "World" + '!';
    assert(s == "Hello, World!");
    s += s;                                  // 자기 자신 이어붙이기
    assert(s == "Hello, World!Hello, World!");

    // push_back 과 용량 증가
    MyString p;
    for (int i = 0; i < 1000; ++i) p.push_back(static_cast<char>('a' + i % 26));
    assert(p.size() == 1000 && p.capacity() >= 1000 && p[26] == 'a');
    p.pop_back();
    assert(p.size() == 999);
    p.shrink_to_fit();
    assert(p.capacity() == 999);

    // 검색 / 부분 문자열
    MyString t("the quick brown fox");
    assert(t.find("quick") == 4);
    assert(t.find('z') == MyString::npos);
    assert(t.rfind('o') == 17);
    assert(t.substr(10, 5) == "brown");
    assert(t.substr(16) == "fox");
    assert(t.starts_with("the") && t.ends_with("fox"));

    // insert / erase / replace
    t.insert(4, "very ");
    assert(t == "the very quick brown fox");
    t.erase(4, 5);
    assert(t == "the quick brown fox");
    t.replace(4, 5, "slow");
    assert(t == "the slow brown fox");

    // 비교
    assert(MyString("abc") < MyString("abd"));
    assert(MyString("ab") < MyString("abc"));
    assert(MyString("b") > MyString("abc"));

    // 예외
    bool thrown = false;
    try { hello.at(10); } catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);

    // 반복자 + STL 알고리즘
    MyString r("dcba");
    std::sort(r.begin(), r.end());
    assert(r == "abcd");
    std::vector<MyString> v{"pear", "apple", "fig"};
    std::sort(v.begin(), v.end());
    assert(v[0] == "apple" && v[2] == "pear");

    // 스트림 입출력
    std::istringstream in("  first second\nwhole line here\n");
    MyString w1, w2, line;
    in >> w1 >> w2;
    in.get();                                // 남은 '\n' 소비
    getline(in, line);
    assert(w1 == "first" && w2 == "second" && line == "whole line here");

    std::ostringstream out;
    out << hello << ' ' << stars;
    assert(out.str() == "Hello ***");

    std::cout << s << '\n' << t << '\n';
    std::cout << "모든 테스트 통과!\n";
    return 0;
}
