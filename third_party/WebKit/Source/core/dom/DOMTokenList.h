/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DOMTokenList_h
#define DOMTokenList_h

#include "base/macros.h"
#include "core/CoreExport.h"
#include "core/dom/QualifiedName.h"
#include "core/dom/SpaceSplitString.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/AtomicString.h"

namespace blink {

class Element;
class ExceptionState;

class CORE_EXPORT DOMTokenList : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static DOMTokenList* Create(Element& element, const QualifiedName& attr) {
    return new DOMTokenList(element, attr);
  }
  virtual ~DOMTokenList() = default;
  virtual void Trace(blink::Visitor*);

  unsigned length() const { return token_set_.size(); }
  const AtomicString item(unsigned index) const;
  bool contains(const AtomicString&) const;
  void add(const Vector<String>&, ExceptionState&);
  void remove(const Vector<String>&, ExceptionState&);
  bool toggle(const AtomicString&, ExceptionState&);
  bool toggle(const AtomicString&, bool force, ExceptionState&);
  bool replace(const AtomicString& token,
               const AtomicString& new_token,
               ExceptionState&);
  bool supports(const AtomicString&, ExceptionState&);
  const AtomicString& value() const { return value_; }
  void setValue(const AtomicString&);
  const AtomicString& toString() const { return value(); }

  // This function should be called when the associated attribute value was
  // updated.
  void DidUpdateAttributeValue(const AtomicString& old_value,
                               const AtomicString& new_value);

  const SpaceSplitString& TokenSet() const { return token_set_; }
  // Add() and Remove() have DCHECK for syntax of the specified token.
  void Add(const AtomicString&);
  void Remove(const AtomicString&);

 protected:
  DOMTokenList(Element& element, const QualifiedName& attr)
      : element_(element), attribute_name_(attr) {}
  Element& GetElement() const { return *element_; }

  virtual bool ValidateTokenValue(const AtomicString&, ExceptionState&) const;

 private:
  void AddTokens(const Vector<String>&);
  void RemoveTokens(const Vector<String>&);
  void UpdateWithTokenSet(const SpaceSplitString&);
  static AtomicString SerializeTokenSet(const SpaceSplitString&);

  SpaceSplitString token_set_;
  AtomicString value_;
  const Member<Element> element_;
  const QualifiedName attribute_name_;
  bool is_in_update_step_ = false;
  DISALLOW_COPY_AND_ASSIGN(DOMTokenList);
};

}  // namespace blink

#endif  // DOMTokenList_h
