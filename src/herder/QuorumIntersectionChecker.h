#pragma once

// Copyright 2019 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "herder/QuorumTracker.h"
#include <atomic>
#include <memory>

namespace caiz
{

class Config;

class QuorumIntersectionChecker
{
  public:
    static std::shared_ptr<QuorumIntersectionChecker>
    create(caiz::QuorumTracker::QuorumMap const& qmap,
           caiz::Config const& cfg, std::atomic<bool>& interruptFlag,
           bool quiet = false);

    static std::set<std::set<NodeID>>
    getIntersectionCriticalGroups(caiz::QuorumTracker::QuorumMap const& qmap,
                                  caiz::Config const& cfg,
                                  std::atomic<bool>& interruptFlag);

    virtual ~QuorumIntersectionChecker(){};
    virtual bool networkEnjoysQuorumIntersection() const = 0;
    virtual size_t getMaxQuorumsFound() const = 0;
    virtual std::pair<std::vector<NodeID>, std::vector<NodeID>>
    getPotentialSplit() const = 0;

    // If any thread sets the atomic interruptFlag passed into any of the above
    // methods, any calculation-in-progress will throw InterruptedException and
    // unwind.
    struct InterruptedException
    {
    };
};
}
