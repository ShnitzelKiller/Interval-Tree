//
// Created by James Noeckel on 2/13/20.
//

#include "IntervalTree.h"
#include <iostream>

int main(int argc, char **argv) {
    std::vector<std::pair<Interval<float>, int>> intervals;
    intervals.reserve(1000);
    for (int i=0; i<1000; i++) {
        intervals.emplace_back(Interval<float>(i-1, i+1), i);
    }

    //construction
    IntervalTree<float, int> tree(intervals.begin(), intervals.end());
    bool success = true;
    {
        auto result = tree.query(50.732);
        std::cout << "result after construction: ";
        for (auto it=result.first; it != result.second; ++it) {
            std::cout << (*it).second << ", ";
        }
        success = success && distance(result.first, result.second) == 2;
        std::cout << std::endl;
    }

    //insert
    {
        tree.insert(Interval<float>(50, 51), 0);
        tree.insert(Interval<float>(49, 52), 1);
        tree.insert(Interval<float>(10, 30), 2);
        auto result = tree.query(50.732);
        std::cout << "result after insertion: ";
        for (auto it=result.first; it != result.second; ++it) {
            std::cout << (*it).second << ", ";
        }
        success = success && distance(result.first, result.second) == 4;
        std::cout << std::endl;
    }

    //clear
    {
        tree.clear();
        auto result = tree.query(50.732);
        std::cout << "result size after clear: " << distance(result.first, result.second) << std::endl;
        success = success && result.first == result.second;
    }

    //build
    {
        tree.build(intervals.begin(), intervals.end());
        auto result = tree.query(50.732);
        std::cout << "result after rebuild: ";
        for (auto it=result.first; it != result.second; ++it) {
            std::cout << (*it).second << ", ";
        }
        std::cout << std::endl;
        success = success && distance(result.first, result.second) == 2;
    }

    //many queries
    {
        std::cout << "some other queries: " << std::endl;
        for (int i=5; i<8; i++) {
            float point = i+0.25f;
            auto result = tree.query(point);
            std::cout << "result at " << point << ": ";
            for (auto it=result.first; it != result.second; ++it) {
                std::cout << (*it).second << ", ";
            }
            std::cout << std::endl;
            success = success && distance(result.first, result.second) == 2;
        }
    }


    return !success;
}
