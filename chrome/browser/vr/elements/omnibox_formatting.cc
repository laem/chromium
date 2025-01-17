// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/vr/elements/omnibox_formatting.h"

#include "ui/gfx/font.h"
#include "ui/gfx/range/range.h"
#include "ui/gfx/render_text.h"
#include "ui/gfx/text_constants.h"

namespace vr {

TextFormatting ConvertClassification(
    const ACMatchClassifications& classifications,
    size_t text_length,
    const ColorScheme& color_scheme) {
  TextFormatting formatting;
  formatting.push_back(TextFormattingAttribute(color_scheme.url_bar_text,
                                               gfx::Range(0, text_length)));

  for (size_t i = 0; i < classifications.size(); ++i) {
    const size_t text_start = classifications[i].offset;
    if (text_start >= text_length)
      break;

    const size_t text_end =
        (i < (classifications.size() - 1))
            ? std::min(classifications[i + 1].offset, text_length)
            : text_length;
    const gfx::Range current_range(text_start, text_end);

    if (classifications[i].style & ACMatchClassification::MATCH) {
      formatting.push_back(
          TextFormattingAttribute(gfx::Font::Weight::BOLD, current_range));
    }

    if (classifications[i].style & ACMatchClassification::URL) {
      formatting.push_back(TextFormattingAttribute(
          gfx::DirectionalityMode::DIRECTIONALITY_AS_URL));
    }

    if (classifications[i].style & ACMatchClassification::URL) {
      formatting.push_back(
          TextFormattingAttribute(color_scheme.hyperlink, current_range));
    } else if (classifications[i].style & ACMatchClassification::INVISIBLE) {
      formatting.push_back(
          TextFormattingAttribute(SK_ColorTRANSPARENT, current_range));
    }
  }
  return formatting;
}

url_formatter::FormatUrlTypes GetVrFormatUrlTypes() {
  return url_formatter::kFormatUrlOmitDefaults |
         url_formatter::kFormatUrlOmitHTTPS |
         url_formatter::kFormatUrlOmitTrivialSubdomains;
}

ElisionParameters GetElisionParameters(const GURL& gurl,
                                       const url::Parsed& parsed,
                                       gfx::RenderText* render_text,
                                       int min_path_pixels) {
  // In situations where there is no host, do not attempt to position the TLD.
  bool allow_offset =
      gurl.IsStandard() && !gurl.SchemeIsFile() && parsed.host.is_nonempty();
  int total_width = render_text->GetContentWidth();

  ElisionParameters result;

  // Find the rightmost extent of the host portion. To safely handle RTL,
  // compute the union of all rendered host segments.
  gfx::Range range(0, parsed.CountCharactersBefore(url::Parsed::PATH, false));
  std::vector<gfx::Rect> rects = render_text->GetSubstringBounds(range);
  gfx::Rect host_bounds;
  for (const auto& rect : rects)
    host_bounds.Union(rect);

  // Choose a right-edge cutoff point.  If there is nothing after the host, then
  // it's the end of the host.  If there is a path, then include a limited
  // portion of the path.
  int path_width = total_width - host_bounds.width();
  int field_width = render_text->display_rect().width();
  int anchor_point =
      host_bounds.width() + std::min(min_path_pixels, path_width);

  if (allow_offset && anchor_point > field_width) {
    result.offset = field_width - anchor_point;
    result.fade_left = true;
  }
  if (total_width + result.offset > field_width) {
    result.fade_right = true;
  }

  return result;
}

}  // namespace vr
