# MyString — C++ string 클래스 직접 구현

`std::string`과 비슷하게 동작하는 헤더 전용(header-only) 문자열 클래스입니다.

## 빌드 및 테스트
```bash
make test        # g++ -std=c++17 로 빌드 후 테스트 실행
```

## 지원 기능
- 생성: 기본, `const char*`, `(const char*, n)`, `(n, c)`, 복사, 이동
- Rule of Five + copy-and-swap 대입 (자기 대입·예외 안전)
- 용량: `size/length/capacity/empty/reserve/resize/clear/shrink_to_fit`
- 접근: `operator[]`, `at`(범위 검사), `front/back`, `c_str/data`
- 반복자: `begin/end/cbegin/cend` (STL 알고리즘 사용 가능)
- 수정: `push_back/pop_back/append/+=/insert/erase/replace/swap`
- 검색: `find/rfind/substr/starts_with/ends_with`
- 비교: `compare`, `== != < > <= >=`
- 연결: `+` (MyString, `const char*`, `char`)
- 입출력: `<<`, `>>`(단어 단위), `getline`

## 설계 메모
- 버퍼는 항상 널 종료되며, 용량이 부족하면 2배씩 증가합니다(최소 15).
- 빈 문자열은 공유 정적 버퍼를 가리켜 기본 생성자·이동 생성자가 할당 없이 `noexcept`입니다.
- 이동된 객체도 유효한 빈 문자열 상태라 그대로 재사용할 수 있습니다.
- `s += s`처럼 자기 자신을 이어붙여도 안전합니다.
