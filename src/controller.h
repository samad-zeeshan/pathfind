// Playback controller. Decouples search stepping from the render loop with a
// time accumulator, so speed is continuous and frame-rate independent.

#pragma once

#include "pathfinder.h"

#include <memory>

class Grid;

class SearchController {
public:
    enum class Mode { Idle, Running, Paused, Done };

    // Swap the algorithm. An active or finished run restarts under the new
    // algorithm so two searches can be compared on the same map.
    void select(std::unique_ptr<Pathfinder> algo);

    // Remember the map and endpoints every restart uses. Invalidates any run.
    void setEndpoints(const Grid& grid, Point start, Point goal);

    void play();
    void pause();
    void togglePlay();
    void stepOnce();      // single expansion, forces Paused
    void runToEnd();
    void reset();         // back to Idle, the next play starts fresh

    void scaleSpeed(double factor);
    double stepsPerSecond() const { return stepsPerSecond_; }

    // Advances the search by accumulated wall time. Call once per frame.
    void update(double dt);

    Mode mode() const { return mode_; }
    const Pathfinder* current() const { return algo_.get(); }

private:
    bool canStart() const { return algo_ != nullptr && grid_ != nullptr; }
    void restart();

    std::unique_ptr<Pathfinder> algo_;
    const Grid* grid_ = nullptr;
    Point start_{}, goal_{};
    Mode mode_ = Mode::Idle;
    double stepsPerSecond_ = 20.0;
    double accumulator_ = 0.0;   // seconds of unspent search time
};
