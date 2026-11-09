// MyString.h - std::string 과 유사한 문자열 클래스 직접 구현
#ifndef MYSTRING_H
#define MYSTRING_H

#include <cstddef>
#include <cstring>
#include <istream>
#include <locale>
#include <ostream>
#include <stdexcept>
#include <utility>

class MyString {
public:
    using size_type = std::size_t;
    using iterator = char*;
    using const_iterator = const char*;
    static constexpr size_type npos = static_cast<size_type>(-1);

    // ===== 생성자 / 소멸자 (Rule of Five) =====
    MyString() noexcept = default;   // 할당 없이 정적 빈 버퍼를 가리킴

    MyString(const char* s) {
        if (!s) throw std::invalid_argument("MyString: null pointer");
        init(s, std::strlen(s));
    }

    MyString(const char* s, size_type n) { init(s, n); }

    MyString(size_type n, char c) : size_(n), capacity_(n), data_(allocate(n)) {
        std::memset(data_, c, n);
        setSize(n);
    }

    MyString(const MyString& other) { init(other.data_, other.size_); }

    MyString(MyString&& other) noexcept
        : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = emptyBuffer();   // 이동된 객체도 유효한 빈 문자열 상태
    }

    ~MyString() { release(); }

    // copy-and-swap: 복사/이동 대입을 하나로 처리, 강한 예외 보장
    MyString& operator=(MyString other) noexcept {
        swap(other);
        return *this;
    }

    MyString& operator=(const char* s) { return *this = MyString(s); }

    // ===== 용량 =====
    size_type size() const noexcept { return size_; }
    size_type length() const noexcept { return size_; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_type newCap) {
        if (newCap <= capacity_) return;
        char* p = allocate(newCap);
        std::memcpy(p, data_, size_ + 1);
        release();
        data_ = p;
        capacity_ = newCap;
    }

    void resize(size_type n, char c = '\0') {
        if (n > size_) {
            reserve(n);
            std::memset(data_ + size_, c, n - size_);
        }
        setSize(n);
    }

    void clear() noexcept { setSize(0); }

    void shrink_to_fit() {
        if (capacity_ == size_) return;
        MyString tmp(*this);
        swap(tmp);
    }

    // ===== 원소 접근 =====
    char& operator[](size_type i) { return data_[i]; }
    const char& operator[](size_type i) const { return data_[i]; }

    char& at(size_type i) {
        if (i >= size_) throw std::out_of_range("MyString::at");
        return data_[i];
    }
    const char& at(size_type i) const {
        if (i >= size_) throw std::out_of_range("MyString::at");
        return data_[i];
    }

    char& front() { return data_[0]; }
    char& back() { return data_[size_ - 1]; }
    const char& front() const { return data_[0]; }
    const char& back() const { return data_[size_ - 1]; }

    const char* c_str() const noexcept { return data_; }
    const char* data() const noexcept { return c_str(); }

    // ===== 반복자 =====
    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }
    const_iterator cbegin() const noexcept { return data_; }
    const_iterator cend() const noexcept { return data_ + size_; }

    // ===== 수정 =====
    void push_back(char c) {
        if (size_ == capacity_) grow(size_ + 1);
        data_[size_++] = c;
        data_[size_] = '\0';
    }

    void pop_back() {
        if (size_ == 0) throw std::out_of_range("MyString::pop_back on empty");
        data_[--size_] = '\0';
    }

    MyString& append(const char* s, size_type n) {
        if (n == 0) return *this;
        if (size_ + n > capacity_) {
            // s 가 자기 자신의 버퍼를 가리킬 수 있으므로(self-append) 새 버퍼에 먼저 복사
            size_type newCap = growthCapacity(size_ + n);
            char* p = allocate(newCap);
            std::memcpy(p, data_, size_);
            std::memcpy(p + size_, s, n);
            release();
            data_ = p;
            capacity_ = newCap;
        } else {
            std::memmove(data_ + size_, s, n);
        }
        size_ += n;
        data_[size_] = '\0';
        return *this;
    }
    MyString& append(const char* s) { return append(s, std::strlen(s)); }
    MyString& append(const MyString& s) { return append(s.data_, s.size_); }
    MyString& append(size_type n, char c) {
        if (n == 0) return *this;
        reserve(size_ + n > capacity_ ? growthCapacity(size_ + n) : capacity_);
        std::memset(data_ + size_, c, n);
        size_ += n;
        data_[size_] = '\0';
        return *this;
    }

    MyString& operator+=(const MyString& s) { return append(s); }
    MyString& operator+=(const char* s) { return append(s); }
    MyString& operator+=(char c) { push_back(c); return *this; }

    MyString& insert(size_type pos, const MyString& s) {
        if (pos > size_) throw std::out_of_range("MyString::insert");
        MyString result;
        result.reserve(size_ + s.size_);
        result.append(data_, pos).append(s).append(data_ + pos, size_ - pos);
        swap(result);
        return *this;
    }

    MyString& erase(size_type pos = 0, size_type len = npos) {
        if (pos > size_) throw std::out_of_range("MyString::erase");
        if (len > size_ - pos) len = size_ - pos;
        if (len == 0) return *this;
        std::memmove(data_ + pos, data_ + pos + len, size_ - pos - len);
        setSize(size_ - len);
        return *this;
    }

    MyString& replace(size_type pos, size_type len, const MyString& s) {
        if (pos > size_) throw std::out_of_range("MyString::replace");
        if (len > size_ - pos) len = size_ - pos;
        MyString result;
        result.reserve(size_ - len + s.size_);
        result.append(data_, pos).append(s).append(data_ + pos + len, size_ - pos - len);
        swap(result);
        return *this;
    }

    void swap(MyString& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
    }

    // ===== 검색 / 부분 문자열 =====
    MyString substr(size_type pos = 0, size_type len = npos) const {
        if (pos > size_) throw std::out_of_range("MyString::substr");
        if (len > size_ - pos) len = size_ - pos;
        return MyString(c_str() + pos, len);
    }

    size_type find(const MyString& s, size_type pos = 0) const noexcept {
        return find(s.c_str(), pos, s.size_);
    }
    size_type find(const char* s, size_type pos = 0) const noexcept {
        return find(s, pos, std::strlen(s));
    }
    size_type find(char c, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < size_; ++i)
            if (data_[i] == c) return i;
        return npos;
    }

    size_type rfind(char c, size_type pos = npos) const noexcept {
        if (size_ == 0) return npos;
        size_type i = pos < size_ ? pos : size_ - 1;
        for (;; --i) {
            if (data_[i] == c) return i;
            if (i == 0) break;
        }
        return npos;
    }

    bool starts_with(const MyString& s) const noexcept {
        return s.size_ <= size_ && std::memcmp(c_str(), s.c_str(), s.size_) == 0;
    }
    bool ends_with(const MyString& s) const noexcept {
        return s.size_ <= size_ &&
               std::memcmp(c_str() + size_ - s.size_, s.c_str(), s.size_) == 0;
    }

    // ===== 비교 =====
    int compare(const MyString& other) const noexcept {
        size_type n = size_ < other.size_ ? size_ : other.size_;
        int r = std::memcmp(c_str(), other.c_str(), n);
        if (r != 0) return r;
        if (size_ < other.size_) return -1;
        if (size_ > other.size_) return 1;
        return 0;
    }

    friend bool operator==(const MyString& a, const MyString& b) noexcept {
        return a.size_ == b.size_ && a.compare(b) == 0;
    }
    friend bool operator!=(const MyString& a, const MyString& b) noexcept { return !(a == b); }
    friend bool operator<(const MyString& a, const MyString& b) noexcept { return a.compare(b) < 0; }
    friend bool operator>(const MyString& a, const MyString& b) noexcept { return b < a; }
    friend bool operator<=(const MyString& a, const MyString& b) noexcept { return !(b < a); }
    friend bool operator>=(const MyString& a, const MyString& b) noexcept { return !(a < b); }

    // ===== 연결 =====
    friend MyString operator+(MyString a, const MyString& b) { a += b; return a; }
    friend MyString operator+(MyString a, char c) { a += c; return a; }
    friend MyString operator+(const char* a, const MyString& b) { MyString r(a); r += b; return r; }

    // ===== 스트림 입출력 =====
    friend std::ostream& operator<<(std::ostream& os, const MyString& s) {
        return os.write(s.c_str(), static_cast<std::streamsize>(s.size_));
    }

    // 공백 기준으로 한 단어 읽기 (std::string 의 operator>> 와 동일한 동작)
    friend std::istream& operator>>(std::istream& is, MyString& s) {
        std::istream::sentry sentry(is);   // 앞쪽 공백 건너뛰기
        if (!sentry) return is;
        s.clear();
        std::streambuf* sb = is.rdbuf();
        for (;;) {
            int ch = sb->sgetc();
            if (ch == std::char_traits<char>::eof()) { is.setstate(std::ios::eofbit); break; }
            if (std::isspace(static_cast<char>(ch), is.getloc())) break;
            s.push_back(static_cast<char>(ch));
            sb->sbumpc();
        }
        if (s.empty()) is.setstate(std::ios::failbit);
        return is;
    }

    // 한 줄 읽기
    friend std::istream& getline(std::istream& is, MyString& s, char delim = '\n') {
        s.clear();
        char ch;
        bool any = false;
        while (is.get(ch)) {
            any = true;
            if (ch == delim) break;
            s.push_back(ch);
        }
        if (any) is.clear(is.rdstate() & ~std::ios::failbit);
        return is;
    }

private:
    size_type size_ = 0;
    size_type capacity_ = 0;
    // capacity_ == 0 이면 정적 빈 버퍼, 그 외에는 capacity_ + 1 바이트 힙 버퍼 (널 종료 포함)
    char* data_ = emptyBuffer();

    static char* emptyBuffer() noexcept {
        static char buf[1] = {'\0'};
        return buf;
    }

    static char* allocate(size_type cap) {
        if (cap == 0) return emptyBuffer();
        char* p = new char[cap + 1];
        p[0] = '\0';
        return p;
    }

    // 길이 설정 + 널 종료. 공유 정적 버퍼에는 쓰지 않음(스레드 안전)
    void setSize(size_type n) noexcept {
        size_ = n;
        if (data_ != emptyBuffer()) data_[n] = '\0';
    }

    void release() noexcept {
        if (data_ != emptyBuffer()) delete[] data_;
    }

    void init(const char* s, size_type n) {
        data_ = allocate(n);
        capacity_ = n;
        std::memcpy(data_, s, n);
        setSize(n);
    }

    size_type growthCapacity(size_type required) const noexcept {
        size_type doubled = capacity_ * 2;
        if (doubled < 15) doubled = 15;
        return doubled > required ? doubled : required;
    }

    void grow(size_type required) { reserve(growthCapacity(required)); }

    size_type find(const char* s, size_type pos, size_type n) const noexcept {
        if (n == 0) return pos <= size_ ? pos : npos;
        if (n > size_) return npos;
        for (size_type i = pos; i + n <= size_; ++i)
            if (std::memcmp(data_ + i, s, n) == 0) return i;
        return npos;
    }
};

inline void swap(MyString& a, MyString& b) noexcept { a.swap(b); }

#endif // MYSTRING_H
