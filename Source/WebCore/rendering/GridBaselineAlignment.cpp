/*
 * Copyright (C) 2018 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "GridBaselineAlignment.h"

#include "AncestorSubgridIterator.h"
#include "BaselineAlignmentInlines.h"
#include "RenderBoxInlines.h"
#include "RenderGrid.h"
#include "RenderStyleConstants.h"

namespace WebCore {

LayoutUnit GridBaselineAlignment::logicalAscentForGridItem(const RenderBox& gridItem, GridAxis alignmentAxis, ItemPosition position) const
{
    auto hasOrthogonalAncestorSubgrids = [&] {
        for (auto& currentAncestorSubgrid : ancestorSubgridsOfGridItem(gridItem, GridTrackSizingDirection::ForRows)) {
            if (currentAncestorSubgrid.isHorizontalWritingMode() != currentAncestorSubgrid.parent()->isHorizontalWritingMode())
                return true;
        }
        return false;
    };

    ExtraMarginsFromSubgrids extraMarginsFromAncestorSubgrids;
    if (alignmentAxis == GridAxis::GridColumnAxis && !hasOrthogonalAncestorSubgrids())
        extraMarginsFromAncestorSubgrids = GridLayoutFunctions::extraMarginForSubgridAncestors(GridTrackSizingDirection::ForRows, gridItem);

    LayoutUnit ascent = ascentForGridItem(gridItem, alignmentAxis, position) + extraMarginsFromAncestorSubgrids.extraTrackStartMargin();
    WTF_ALWAYS_LOG("Ascent for item  - " << gridItem << " - " << ascent);
    return ascent;
    //return (isDescentBaselineForGridItem(gridItem, alignmentAxis) || position == ItemPosition::LastBaseline) ? descentForGridItem(gridItem, ascent, alignmentAxis, extraMarginsFromAncestorSubgrids) : ascent;
}

LayoutUnit GridBaselineAlignment::ascentForGridItem(const RenderBox& gridItem, GridAxis alignmentAxis, ItemPosition position) const
{
    ASSERT(position == ItemPosition::Baseline || position == ItemPosition::LastBaseline);
    auto gridItemMargin = alignmentAxis == GridAxis::GridColumnAxis ? gridItem.marginBlockStart(m_writingMode) : gridItem.marginInlineStart(m_writingMode);
    auto& parentStyle = gridItem.parent()->style();

    auto shouldSynthesizeBaseline = [&] {
        // idk if this is even the right thing or if it should be the opposite.
        if (alignmentAxis == GridAxis::GridRowAxis)
            return isHorizontalWritingMode(m_writingMode) == isHorizontalWritingMode(gridItem.style().writingMode());
        return isHorizontalWritingMode(m_writingMode) != isHorizontalWritingMode(gridItem.style().writingMode());
    }();

    auto alignmentContextDirection = [&] {
        if (parentStyle.isHorizontalWritingMode())
            return alignmentAxis == GridAxis::GridRowAxis ? LineDirectionMode::VerticalLine : LineDirectionMode::HorizontalLine;
        return alignmentAxis == GridAxis::GridRowAxis ? LineDirectionMode::HorizontalLine : LineDirectionMode::VerticalLine;
    };

    auto baseline = [&] {
        if (shouldSynthesizeBaseline)
            return synthesizedBaseline(gridItem, parentStyle, alignmentContextDirection(), BaselineSynthesisEdge::BorderBox);
        if (auto baselineFromRenderer = position == ItemPosition::Baseline ? gridItem.firstLineBaseline() : gridItem.lastLineBaseline())
            return baselineFromRenderer.value();
        return synthesizedBaseline(gridItem, parentStyle, alignmentContextDirection(), BaselineSynthesisEdge::BorderBox);
    }();

    auto& renderGrid = *downcast<RenderGrid>(gridItem.parent());

    auto writingModeForBaselineAlignment = GridLayoutFunctions::writingModeForBaselineAlignment(renderGrid, gridItem, gridDirectionForAxis(alignmentAxis));
    if (alignmentAxis == GridAxis::GridRowAxis && isFlippedWritingMode(writingModeForBaselineAlignment) == renderGrid.style().isLeftToRightDirection())
        baseline = GridLayoutFunctions::borderBoxLogicalSizeForGridItem(renderGrid, GridTrackSizingDirection::ForColumns, gridItem) - baseline;
        

    if (alignmentAxis == GridAxis::GridColumnAxis && writingModeToBlockFlowDirection(renderGrid.style().writingMode()) != writingModeToBlockFlowDirection(writingModeForBaselineAlignment))
        baseline = GridLayoutFunctions::borderBoxLogicalSizeForGridItem(renderGrid, GridTrackSizingDirection::ForRows, gridItem) - baseline;

    return gridItemMargin + baseline;
}

LayoutUnit GridBaselineAlignment::descentForGridItem(const RenderBox& gridItem, LayoutUnit ascent, GridAxis alignmentAxis, ExtraMarginsFromSubgrids extraMarginsFromAncestorSubgrids) const
{
    ASSERT(!gridItem.needsLayout());
    if (isParallelToAlignmentAxisForGridItem(gridItem, alignmentAxis))
        return extraMarginsFromAncestorSubgrids.extraTotalMargin() + gridItem.marginLogicalHeight() + gridItem.logicalHeight() - ascent;
    return gridItem.marginLogicalWidth() + gridItem.logicalWidth() - ascent;
}

bool GridBaselineAlignment::isDescentBaselineForGridItem(const RenderBox& gridItem, GridAxis alignmentAxis) const
{
    return isVerticalAlignmentContext(alignmentAxis)
        && ((gridItem.style().isFlippedBlocksWritingMode() && !isFlippedWritingMode(m_writingMode))
            || (gridItem.style().isFlippedLinesWritingMode() && isFlippedWritingMode(m_writingMode)));
}

bool GridBaselineAlignment::isVerticalAlignmentContext(GridAxis alignmentAxis) const
{
    return alignmentAxis == GridAxis::GridRowAxis ? isHorizontalWritingMode(m_writingMode) : !isHorizontalWritingMode(m_writingMode);
}

bool GridBaselineAlignment::isOrthogonalGridItemForBaseline(const RenderBox& gridItem) const
{
    return isHorizontalWritingMode(m_writingMode) != gridItem.isHorizontalWritingMode();
}

bool GridBaselineAlignment::isParallelToAlignmentAxisForGridItem(const RenderBox& gridItem, GridAxis alignmentAxis) const
{
    return alignmentAxis == GridAxis::GridColumnAxis ? !isOrthogonalGridItemForBaseline(gridItem) : isOrthogonalGridItemForBaseline(gridItem);
}

const BaselineGroup& GridBaselineAlignment::baselineGroupForGridItem(ItemPosition preference, unsigned sharedContext, const RenderBox& gridItem, GridAxis alignmentAxis) const
{
    ASSERT(isBaselinePosition(preference));
    auto& baselineAlignmentStateMap = alignmentAxis == GridAxis::GridRowAxis ? m_rowAxisBaselineAlignmentStates : m_colAxisBaselineAlignmentStates;
    auto* baselineAlignmentState = baselineAlignmentStateMap.get(sharedContext);
    ASSERT(baselineAlignmentState);
    return baselineAlignmentState->sharedGroup(gridItem, preference);
}

void GridBaselineAlignment::updateBaselineAlignmentContext(ItemPosition preference, unsigned sharedContext, const RenderBox& gridItem, GridAxis alignmentAxis)
{
    ASSERT(isBaselinePosition(preference));
    ASSERT(!gridItem.needsLayout());

    // Determine Ascent and Descent values of this grid item with respect to
    // its grid container.
    LayoutUnit ascent = logicalAscentForGridItem(gridItem, alignmentAxis, preference);
    // Looking up for a shared alignment context perpendicular to the
    // alignment axis.
    auto& baselineAlignmentStateMap = alignmentAxis == GridAxis::GridRowAxis ? m_rowAxisBaselineAlignmentStates : m_colAxisBaselineAlignmentStates;
    // Looking for a compatible baseline-sharing group.
    if (auto* baselineAlignmentStateSearch = baselineAlignmentStateMap.get(sharedContext))
        baselineAlignmentStateSearch->updateSharedGroup(gridItem, preference, ascent);
    else
        baselineAlignmentStateMap.add(sharedContext, makeUnique<BaselineAlignmentState>(gridItem, preference, ascent));
}

LayoutUnit GridBaselineAlignment::baselineOffsetForGridItem(ItemPosition preference, unsigned sharedContext, const RenderBox& gridItem, GridAxis alignmentAxis) const
{
    ASSERT(isBaselinePosition(preference));
    auto& group = baselineGroupForGridItem(preference, sharedContext, gridItem, alignmentAxis);
    if (group.computeSize() > 1) {
        auto ascent = logicalAscentForGridItem(gridItem, alignmentAxis, preference);
        return group.maxAscent() - ascent;
    }
    return LayoutUnit();
}

void GridBaselineAlignment::clear(GridAxis alignmentAxis)
{
    if (alignmentAxis == GridAxis::GridRowAxis)
        m_rowAxisBaselineAlignmentStates.clear();
    else
        m_colAxisBaselineAlignmentStates.clear();
}

} // namespace WebCore
