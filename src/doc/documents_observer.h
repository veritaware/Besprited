// Document Library
// Aseprite  | Copyright (C) 2001-2014 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace doc
{
class Document;

class CreateDocumentArgs
{
public:
  CreateDocumentArgs()
    : m_doc(NULL)
  {
  }
  Document* document() { return m_doc; }
  void setDocument(Document* doc) { m_doc = doc; }

private:
  Document* m_doc;
};

class DocumentsObserver
{
public:
  virtual ~DocumentsObserver() {}
  virtual void onCreateDocument(CreateDocumentArgs* args) {}
  virtual void onAddDocument(Document* doc) {}
  virtual void onRemoveDocument(Document* doc) {}
};

} // namespace doc
