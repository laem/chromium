/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGResourcesCycleSolver_h
#define SVGResourcesCycleSolver_h

#include "base/macros.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/HashSet.h"

namespace blink {

class LayoutObject;
class LayoutSVGResourceContainer;
class SVGResources;

// This class traverses the graph formed by SVGResources of
// LayoutObjects, maintaining the active path as LayoutObjects are
// visited. It also maintains a cache of sub-graphs that has already
// been visited and that does not contain any cycles.
class SVGResourcesCycleSolver {
  STACK_ALLOCATED();

 public:
  SVGResourcesCycleSolver(LayoutObject&);
  ~SVGResourcesCycleSolver();

  // Traverse the graph starting at the resource container
  // passed. Returns true if a cycle is detected.
  bool FindCycle(LayoutSVGResourceContainer*);

  typedef HashSet<LayoutSVGResourceContainer*> ResourceSet;

 private:
  bool TraverseResourceContainer(LayoutSVGResourceContainer*);
  bool TraverseResources(LayoutObject&);
  bool TraverseResources(SVGResources*);

  LayoutObject& layout_object_;

  ResourceSet active_resources_;
  ResourceSet dag_cache_;
  DISALLOW_COPY_AND_ASSIGN(SVGResourcesCycleSolver);
};

}  // namespace blink

#endif
