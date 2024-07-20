#define _USE_MATH_DEFINES

#include <cmath>
#include <functional>
#include <sstream>
#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>

#include "../src/collision_detector.h"

namespace Catch {
    template<>
    struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

        return tmp.str();
    }
    };
}

class ItemGathererTest : public collision_detector::ItemGathererProvider {
public:

    ItemGathererTest(std::vector<collision_detector::Item> items, std::vector<collision_detector::Gatherer> gatherers) 
    : items_(items)
    , gatherers_(gatherers)
    {}

    size_t ItemsCount() const override {
        return items_.size();
    }

    collision_detector::Item GetItem(size_t idx) const override {
        return items_.at(idx);
    }

    size_t GatherersCount() const override {
        return gatherers_.size();
    }

    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gatherers_.at(idx);
    }

private:
    std::vector<collision_detector::Item> items_;
    std::vector<collision_detector::Gatherer> gatherers_;
};

template <typename Range, typename Predicate>
struct EqualEventsMatcher : Catch::Matchers::MatcherGenericBase {
    EqualEventsMatcher(Range& range, Predicate predicate)
        : range_{range}
        , predicate_(predicate) 
    {}

    EqualEventsMatcher(EqualEventsMatcher&&) = default;

    template <typename OtherRange>
    bool match(OtherRange other) const {
        using std::begin;
        using std::end;
        
        return std::equal(begin(range_), end(range_), begin(other), end(other), predicate_);
    }

    std::string describe() const override {
        return "Non equal: " + Catch::rangeToString(range_);
    }

private:
    Range& range_;
    Predicate predicate_;
}; 

auto equal_event = [](collision_detector::GatheringEvent lhs, collision_detector::GatheringEvent rhs){
    if(lhs.gatherer_id != rhs.gatherer_id || lhs.item_id != rhs.item_id){
        return false;
    }
    static const double eps = 1e-10;
    if(std::abs(lhs.sq_distance - rhs.sq_distance) > eps){
        return false;
    }
    if(std::abs(lhs.time - rhs.time) > eps){
        return false;
    }
    return true;
};

template<typename Range, typename Predicate>
EqualEventsMatcher<Range, Predicate> EqualEvents(Range& range, Predicate predicate) {
    return EqualEventsMatcher<Range, Predicate>{range, predicate};
} 

SCENARIO("collision detector tests", "[Collision Detector]") {

    WHEN("1 gatherer and 0 items") {
        ItemGathererTest test{{}, {{0, {1.5, 5.5},{8.4, 5.5}, 1}}};
        THEN("no events") {
            auto events = FindGatherEvents(test);
            CHECK(events.empty());
        }
    }
    WHEN("0 gatherer and 2 item") {
        ItemGathererTest test{{{{3.8, 6.1}, 0}, {{7.1, 5.2}, 0}}, {}};
        THEN("no events") {
            auto events = FindGatherEvents(test);
            CHECK(events.empty());
        }
    }
    WHEN("1 gatherer and 2 items in range") {
        ItemGathererTest test{{{{3.57, 6.1}, 0}, {{5.64, 5.2}, 0}}, {{0, {1.5, 5.5},{8.4, 5.5}, 1}}};
        THEN("all items collected") {
            auto events = FindGatherEvents(test);
            std::vector<collision_detector::GatheringEvent> gat_events = {{0, 0, 0.6 * 0.6, 0.3}, {1, 0, 0.3 * 0.3, 0.6}};
            CHECK(events.size() == 2);
            CHECK_THAT(events, EqualEvents(gat_events, equal_event));
        }
    }
    WHEN("1 gatherer and 2 items out range") {
        ItemGathererTest test{{{{3.8, 6.6}, 0}, {{7.1, 4.2}, 0}}, {{0, {1.5, 5.5},{8.4, 5.5}, 1}}};
        THEN("no collected items") {
            auto events = FindGatherEvents(test);
            CHECK(events.empty());
        }
    }
    WHEN("2 gatherer and 1 item"){
        ItemGathererTest test{{{{7.02, 4.55}, 0}}, {{0, {1.5, 5.5},{8.4, 5.5}, 1}, {1, {6.5, 3.5},{6.5, 7.}, 1}}};

        THEN("item collected by faster gatherer") {
            auto events = FindGatherEvents(test);
            std::vector<collision_detector::GatheringEvent> gat_events = {{0, 1, 0.52 * 0.52, 0.3}, {0, 0, 0.95 * 0.95, 0.8}};
            CHECK(events.size() == 2);
            CHECK(events.front().gatherer_id == 1);
            CHECK_THAT(events, EqualEvents(gat_events, equal_event));
        }   
    }
    WHEN("1 gatherer and 4 items (3 - in range, 1 - out range)"){
        ItemGathererTest test{{{{5., 5.5}, 0}, {{9., 4.5}, 0}, {{2., 5.2}, 0}, {{7., 3.9}, 0}}, {{0, {0, 5.},{10., 5.}, 1}}};
        THEN("events in right order and correct values") {            
            auto events = FindGatherEvents(test);
            std::vector<collision_detector::GatheringEvent> gat_events = {{2, 0, 0.2 * 0.2, 0.2}, {0, 0, 0.5 * 0.5, 0.5}, {1, 0, 0.5 * 0.5, 0.9}};
            CHECK(events.size() == 3);
            CHECK_THAT(events, EqualEvents(gat_events, equal_event));
        }
    }
    WHEN("non-horizonal and non-vertical direction gatherer"){
        ItemGathererTest test{{{{2., 2.5}, 0}}, {{0, {1., 1.},{3., 3.}, 0.5}}}; 
        THEN("right item must collected") {            
            auto events = FindGatherEvents(test);
            CHECK(events.size() == 1);
        }
    }
    WHEN("item width non 0") {
        ItemGathererTest test{{{{3.57, 6.6}, 0.5}}, {{0, {1.5, 5.5},{8.4, 5.5}, 1}}};
        THEN("event counted") {
            auto events = FindGatherEvents(test);
            std::vector<collision_detector::GatheringEvent> gat_events = {{0, 0, 1.1 * 1.1, 0.3}};
            CHECK(events.size() == 1);
            CHECK_THAT(events, EqualEvents(gat_events, equal_event));
        }
    }
}