// Copyright 2016 the chromium authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/cssom/StylePropertyMapReadOnly.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/css/CSSCustomPropertyDeclaration.h"
#include "core/css/CSSValueList.h"
#include "core/css/CSSVariableReferenceValue.h"
#include "core/css/cssom/CSSStyleValue.h"
#include "core/css/cssom/CSSUnparsedValue.h"
#include "core/css/cssom/CSSUnsupportedStyleValue.h"
#include "core/css/cssom/StyleValueFactory.h"
#include "core/css/properties/css_property.h"
#include "core/css_property_names.h"
#include "core/style_property_shorthand.h"

namespace blink {

namespace {

class StylePropertyMapIterationSource final
    : public PairIterable<String, CSSStyleValueVector>::IterationSource {
 public:
  explicit StylePropertyMapIterationSource(
      HeapVector<StylePropertyMapReadOnly::StylePropertyMapEntry> values)
      : index_(0), values_(values) {}

  bool Next(ScriptState*,
            String& key,
            CSSStyleValueVector& value,
            ExceptionState&) override {
    if (index_ >= values_.size())
      return false;

    const StylePropertyMapReadOnly::StylePropertyMapEntry& pair =
        values_.at(index_++);
    key = pair.first;
    value = pair.second;
    return true;
  }

  virtual void Trace(blink::Visitor* visitor) {
    visitor->Trace(values_);
    PairIterable<String, CSSStyleValueVector>::IterationSource::Trace(visitor);
  }

 private:
  size_t index_;
  const HeapVector<StylePropertyMapReadOnly::StylePropertyMapEntry> values_;
};

}  // namespace

CSSStyleValue* StylePropertyMapReadOnly::get(const String& property_name,
                                             ExceptionState& exception_state) {
  const CSSPropertyID property_id = cssPropertyID(property_name);
  if (property_id == CSSPropertyInvalid) {
    exception_state.ThrowTypeError("Invalid propertyName: " + property_name);
    return nullptr;
  }

  DCHECK(isValidCSSPropertyID(property_id));
  const CSSProperty& property = CSSProperty::Get(property_id);
  if (property.IsShorthand())
    return GetShorthandProperty(property);

  const CSSValue* value = (property_id == CSSPropertyVariable)
                              ? GetCustomProperty(AtomicString(property_name))
                              : GetProperty(property_id);
  if (!value)
    return nullptr;

  if (property.IsRepeated()) {
    CSSStyleValueVector values =
        StyleValueFactory::CssValueToStyleValueVector(property_id, *value);
    return values.IsEmpty() ? nullptr : values[0];
  }

  return StyleValueFactory::CssValueToStyleValue(property_id, *value);
}

CSSStyleValueVector StylePropertyMapReadOnly::getAll(
    const String& property_name,
    ExceptionState& exception_state) {
  CSSPropertyID property_id = cssPropertyID(property_name);
  if (property_id == CSSPropertyInvalid) {
    exception_state.ThrowTypeError("Invalid propertyName: " + property_name);
    return CSSStyleValueVector();
  }

  DCHECK(isValidCSSPropertyID(property_id));
  const CSSProperty& property = CSSProperty::Get(property_id);
  if (property.IsShorthand()) {
    CSSStyleValueVector values;
    if (CSSStyleValue* value = GetShorthandProperty(property))
      values.push_back(value);
    return values;
  }

  const CSSValue* value = (property_id == CSSPropertyVariable)
                              ? GetCustomProperty(AtomicString(property_name))
                              : GetProperty(property_id);
  if (!value)
    return CSSStyleValueVector();

  return StyleValueFactory::CssValueToStyleValueVector(property_id, *value);
}

bool StylePropertyMapReadOnly::has(const String& property_name,
                                   ExceptionState& exception_state) {
  return !getAll(property_name, exception_state).IsEmpty();
}

StylePropertyMapReadOnly::IterationSource*
StylePropertyMapReadOnly::StartIteration(ScriptState*, ExceptionState&) {
  HeapVector<StylePropertyMapReadOnly::StylePropertyMapEntry> result;

  ForEachProperty([&result](const String& property_name,
                            const CSSValue& css_value) {
    const auto property_id = cssPropertyID(property_name);

    auto values =
        StyleValueFactory::CssValueToStyleValueVector(property_id, css_value);
    result.emplace_back(property_name, std::move(values));
  });

  return new StylePropertyMapIterationSource(result);
}

CSSStyleValue* StylePropertyMapReadOnly::GetShorthandProperty(
    const CSSProperty& property) {
  DCHECK(property.IsShorthand());
  const auto serialization = SerializationForShorthand(property);
  if (serialization.IsEmpty())
    return nullptr;
  return CSSUnsupportedStyleValue::Create(property.PropertyID(), serialization);
}

}  // namespace blink
