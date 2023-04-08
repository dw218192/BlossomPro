#pragma once

struct ControlPointArray {
private:
    std::vector<double> m_x;
    std::vector<double> m_y;
public:

    template<typename T>
    struct BaseIterator {
        friend struct ControlPointArray;

        using data_reference_type = typename std::iterator_traits<T>::reference;
        using value_type = std::pair<data_reference_type, data_reference_type> const;
        using reference = value_type;
        using pointer = struct Proxy {
            value_type data;
            value_type* operator->() const { return &data; }
        };
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        BaseIterator() = delete;
        BaseIterator(T const& x_it, T const& y_it)
            : m_x_it(x_it), m_y_it(y_it) {}
        reference operator*() const { return { *m_x_it, *m_y_it }; }
        pointer operator->() const { return { { *m_x_it, *m_y_it } }; }
        BaseIterator& operator++() { ++m_x_it; ++m_y_it; return *this; }
        BaseIterator operator++(int) { auto copy = *this; ++(*this); return copy; }
        BaseIterator& operator--() { --m_x_it; --m_y_it; return *this; }
        BaseIterator operator--(int) { auto copy = *this; --(*this); return copy; }
        BaseIterator& operator+=(difference_type n) { m_x_it += n; m_y_it += n; return *this; }
        BaseIterator operator+(difference_type n) const { auto copy = *this; copy += n; return copy; }
        BaseIterator& operator-=(difference_type n) { return (*this) += -n; }
        BaseIterator operator-(difference_type n) const { auto copy = *this; copy -= n; return copy; }
        difference_type operator-(const BaseIterator& other) const { return m_x_it - other.m_x_it; }
        reference operator[](difference_type n) const { return *(*this + n); }
        bool operator==(const BaseIterator& other) const { return m_x_it == other.m_x_it; }
        bool operator!=(const BaseIterator& other) const { return !(*this == other); }
        bool operator<(const BaseIterator& other) const { return m_x_it < other.m_x_it; }
        bool operator<=(const BaseIterator& other) const { return m_x_it <= other.m_x_it; }
        bool operator>(const BaseIterator& other) const { return m_x_it > other.m_x_it; }
        bool operator>=(const BaseIterator& other) const { return m_x_it >= other.m_x_it; }

    private:
        T m_x_it;
        T m_y_it;
    };

    using Iterator = BaseIterator<typename std::vector<double>::iterator>;
    using ConstIterator = BaseIterator<typename std::vector<double>::const_iterator>;

    Iterator begin() { return { m_x.begin(), m_y.begin() }; }
    Iterator end() { return { m_x.end(), m_y.end() }; }
    ConstIterator begin() const { return { m_x.cbegin(), m_y.cbegin() }; }
    ConstIterator end() const  { return { m_x.cend(), m_y.cend() }; }
    ConstIterator cbegin() const { return { m_x.cbegin(), m_y.cbegin() }; }
    ConstIterator cend() const { return { m_x.cend(), m_y.cend() }; }

    Iterator::reference operator[](size_t i) {
        return begin()[static_cast<Iterator::difference_type>(i)];
    }
    ConstIterator::reference operator[](size_t i) const {
        return cbegin()[static_cast<ConstIterator::difference_type>(i)];
    } 
    void add(double x, double y) {
        m_x.push_back(x);
        m_y.push_back(y);
    }
    void insert(Iterator it, double x, double y) {
        m_x.insert(it.m_x_it, x);
        m_y.insert(it.m_y_it, y);
    }
    std::vector<double> const& getX() const {
        return m_x;
    }
    std::vector<double> const& getY() const {
        return m_y;
    }
    size_t size() const {
        return m_x.size();
    }
    void clear() {
        m_x.clear();
        m_y.clear();
    }
    bool operator==(ControlPointArray const& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }
    bool operator!=(ControlPointArray const& other) const {
        return !(*this == other);
    }
};
