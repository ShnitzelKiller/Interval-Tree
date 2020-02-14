//
// Created by James Noeckel on 2/12/20.
//

#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include <limits>
#include <numeric>

template<typename T>
struct Interval {
    Interval(const T &start_p, const T &end_p) : start(start_p), end(end_p) {}

    T start;
    T end;
};

/**
 * An interval tree allows speedy intersection of a point on the number line with a collection of (possibly overlapping) intervals.
 * This structure acts as a tree multi-map with intersecting intervals as the keys.
 * @tparam T floating point type used by intervals
 * @tparam V stored value type
 */
template<typename T, typename V>
class IntervalTree {
public:
    IntervalTree() = default;
    IntervalTree(const IntervalTree &other);
    IntervalTree(IntervalTree &&other) noexcept;
    IntervalTree &operator=(const IntervalTree &other);
    IntervalTree &operator=(IntervalTree &&other) noexcept;
    explicit IntervalTree(const std::vector<std::pair<Interval<T>, V>> &intervals);

    /**
     * Compute an interval tree with the given list of interval-value pairs.
     * @param intervals a collection of interval-value pairs where each `Interval(a, b)` represents the half-open interval [a, b).
     * Consequently, b must be greater than a.
     */
    void build(const std::vector<std::pair<Interval<T>, V>> &intervals);

    /**
     * Insert a single new value at the given interval. Note that this is a non-rebalancing tree, so if constructing a new tree
     * from a large number of elements, use build().
     * @param interval new interval (with positive length)
     * @param value value to store at the new interval
     */
    void insert(const Interval<T> &interval, const V &value);

    /**
     * Find all intervals intersecting with the query point.
     * @param val query point
     * @return Collection of interval-value pairs whose interval contains the query point.
     */
    std::vector<std::pair<Interval<T>, V>> query(T val) const;

    /**
     * Clears all data and resets the interval tree
     */
    void clear();

private:
    struct TreeNode;
    std::unique_ptr<TreeNode> root_;
};

template <typename T, typename V>
IntervalTree<T, V>::IntervalTree(const IntervalTree &other) {
    root_ = other.root_->clone();
}
template <typename T, typename V>
IntervalTree<T, V>::IntervalTree(IntervalTree &&other) noexcept {
    root_ = std::move(other.root_);
}
template <typename T, typename V>
IntervalTree<T, V> &IntervalTree<T, V>::operator=(const IntervalTree &other) {
    if (this != &other) {
        root_ = other.root_->clone();
    }
    return *this;
}
template <typename T, typename V>
IntervalTree<T, V> &IntervalTree<T, V>::operator=(IntervalTree &&other) noexcept {
    if (this != &other) {
        root_ = std::move(other.root_);
    }
    return *this;
}

template <typename T, typename V>
IntervalTree<T, V>::IntervalTree(const std::vector<std::pair<Interval<T>, V>> &intervals) {
    build(intervals);
}

template<typename T, typename V>
void IntervalTree<T, V>::build(const std::vector<std::pair<Interval<T>, V>> &intervals) {
    root_ = std::make_unique<TreeNode>(intervals);
}

template <typename T, typename V>
void IntervalTree<T, V>::insert(const Interval<T> &interval, const V &value) {
    if (root_) {
        root_->insert(interval, value);
    } else {
        root_ = std::make_unique<TreeNode>(interval, value);
    }
}

template<typename T, typename V>
std::vector<std::pair<Interval<T>, V>> IntervalTree<T, V>::query(T val) const {
    std::vector<std::pair<Interval<T>, V>> results;
    if (root_) {
        root_->query(val, results);
    }
    return results;
}

template <typename T, typename V>
void IntervalTree<T, V>::clear() {
    root_.reset(nullptr);
}

template<typename T, typename V>
struct IntervalTree<T, V>::TreeNode {
     struct IntervalComp {
         IntervalComp(const std::vector<std::pair<Interval<T>, V>> &intervals, bool sort_by_start) : intervals_(intervals), sort_by_start_(sort_by_start) {}
         bool operator()(size_t a, size_t b) const {
             if (sort_by_start_) {
                 return intervals_[a].first.start < intervals_[b].first.start;
             } else {
                 return intervals_[a].first.end < intervals_[b].first.end;
             }
         }
         const std::vector<std::pair<Interval<T>, V>> &intervals_;
         bool sort_by_start_;
     };

     void sort() {
         if (index_sorted_by_start_.size() < center_.size()) {
             size_t n = index_sorted_by_start_.size();
             index_sorted_by_start_.resize(center_.size());
             index_sorted_by_end_.resize(center_.size());
             std::iota(index_sorted_by_start_.begin() + n, index_sorted_by_start_.end(), n);
             std::iota(index_sorted_by_end_.begin() + n, index_sorted_by_end_.end(), n);
         }
         std::sort(index_sorted_by_start_.begin(), index_sorted_by_start_.end(), IntervalComp(center_, true));
         std::sort(index_sorted_by_end_.begin(), index_sorted_by_end_.end(), IntervalComp(center_, false));
     }

     TreeNode() = default;

    /**
     * Recursively construct an interval tree rooted at this node
     * @param intervals intervals to distribute among the nodes
     */
    explicit TreeNode(const std::vector<std::pair<Interval<T>, V>> &intervals) {
        T t_min = std::numeric_limits<T>::max();
        T t_max = std::numeric_limits<T>::lowest();
        for (const auto &interval : intervals) {
            t_min = std::min(t_min, interval.first.start);
            t_max = std::max(t_max, interval.first.end);
        }
        x_center_ = (t_min + t_max) / 2;
        if (x_center_ == t_max) x_center_ = t_min; //circumvents a numerical issue that leads to infinite depth
        std::vector<std::pair<Interval<T>, V>> left;
        std::vector<std::pair<Interval<T>, V>> right;
        for (const auto &interval : intervals) {
            if (interval.first.end <= x_center_) {
                left.push_back(interval);
            } else if (interval.first.start > x_center_) {
                right.push_back(interval);
            } else {
                center_.push_back(interval);
            }
        }
        sort();
        if (!left.empty()) {
            left_ = std::make_unique<TreeNode>(left);
        }
        if (!right.empty()) {
            right_ = std::make_unique<TreeNode>(right);
        }
    }

    TreeNode(const Interval<T> &interval, const V &value) {
        x_center_ = (interval.start + interval.end)/2;
        if (x_center_ == interval.end) x_center_ = interval.start;
        center_.emplace_back(interval, value);
        index_sorted_by_start_.push_back(0);
        index_sorted_by_end_.push_back(0);
    }

    void insert(const Interval<T> &interval, const V &value) {
        if (interval.end <= x_center_) {
            if (left_) {
                left_->insert(interval, value);
            } else {
                left_ = std::make_unique<TreeNode>(interval, value);
            }
        } else if (interval.start > x_center_) {
            if (right_) {
                right_->insert(interval, value);
            } else {
                right_ = std::make_unique<TreeNode>(interval, value);
            }
        } else {
            center_.emplace_back(interval, value);
            sort();
        }
    }

    void query(T val, std::vector<std::pair<Interval<T>, V>> &results) const {
        if (val <= x_center_) {
            for (auto it = index_sorted_by_start_.begin(); it != index_sorted_by_start_.end(); it++) {
                if (center_[*it].first.start <= val) {
                    results.push_back(center_[*it]);
                } else {
                    break;
                }
            }
            if (left_) {
                left_->query(val, results);
            }
        } else {
            for (auto it = index_sorted_by_end_.rbegin(); it != index_sorted_by_end_.rend(); it++) {
                if (center_[*it].first.end > val) {
                    results.push_back(center_[*it]);
                } else {
                    break;
                }
            }
            if (right_) {
                right_->query(val, results);
            }
        }
    }

    std::unique_ptr<TreeNode> clone() const {
        auto root = std::make_unique<TreeNode>();
        root->center_ = center_;
        root->index_sorted_by_start_ = index_sorted_by_start_;
        root->index_sorted_by_end_ = index_sorted_by_end_;
        root->x_center_ = x_center_;
        if (left_) {
            root->left_ = std::make_unique<TreeNode>();
            root->left_ = left_->clone();
        }
        if (right_) {
            root->right_ = std::make_unique<TreeNode>();
            root->right_ = right_->clone();
        }
        return root;
    }

    std::unique_ptr<TreeNode> left_, right_;
    std::vector<std::pair<Interval<T>, V>> center_;
    std::vector<size_t> index_sorted_by_start_;
    std::vector<size_t> index_sorted_by_end_;
    T x_center_;
};