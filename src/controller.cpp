// SearchController implementation.

#include "controller.h"

#include <algorithm>

namespace {

constexpr double kMinStepsPerSecond = 0.5;
constexpr double kMaxStepsPerSecond = 4096.0;

// Cap the backlog so a stalled frame (tab switch, window drag) does not dump
// thousands of steps at once when rendering resumes.
constexpr double kMaxBacklogSeconds = 0.25;

}  // namespace

void SearchController::select(std::unique_ptr<Pathfinder> algo) {
    const bool wasActive = mode_ != Mode::Idle;
    algo_ = std::move(algo);
    mode_ = Mode::Idle;
    if (wasActive && canStart()) restart();
}

void SearchController::setEndpoints(const Grid& grid, Point start, Point goal) {
    grid_ = &grid;
    start_ = start;
    goal_ = goal;
    mode_ = Mode::Idle;
}

void SearchController::restart() {
    algo_->init(*grid_, start_, goal_);
    accumulator_ = 0.0;
    // Unreachable endpoints fail during init, before the first step.
    mode_ = algo_->status() == SearchStatus::Running ? Mode::Running : Mode::Done;
}

void SearchController::play() {
    if (!canStart()) return;
    if (mode_ == Mode::Idle || mode_ == Mode::Done) {
        restart();
    } else if (mode_ == Mode::Paused) {
        mode_ = Mode::Running;
    }
}

void SearchController::pause() {
    if (mode_ == Mode::Running) mode_ = Mode::Paused;
}

void SearchController::togglePlay() {
    if (mode_ == Mode::Running) pause();
    else play();
}

void SearchController::stepOnce() {
    if (!canStart()) return;
    if (mode_ == Mode::Idle || mode_ == Mode::Done) {
        restart();
        if (mode_ == Mode::Done) return;
    }
    mode_ = algo_->step() == SearchStatus::Running ? Mode::Paused : Mode::Done;
}

void SearchController::runToEnd() {
    if (!canStart()) return;
    if (mode_ == Mode::Idle) restart();
    if (mode_ == Mode::Done) return;
    algo_->runToEnd();
    mode_ = Mode::Done;
}

void SearchController::reset() {
    mode_ = Mode::Idle;
}

void SearchController::scaleSpeed(double factor) {
    stepsPerSecond_ = std::min(kMaxStepsPerSecond,
                               std::max(kMinStepsPerSecond, stepsPerSecond_ * factor));
}

void SearchController::update(double dt) {
    if (mode_ != Mode::Running) return;
    const double interval = 1.0 / stepsPerSecond_;
    // The cap must never fall below one interval or slow speeds would starve.
    accumulator_ = std::min(accumulator_ + dt, std::max(interval, kMaxBacklogSeconds));
    while (accumulator_ >= interval && mode_ == Mode::Running) {
        accumulator_ -= interval;
        if (algo_->step() != SearchStatus::Running) mode_ = Mode::Done;
    }
}
