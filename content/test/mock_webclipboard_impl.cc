// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/test/mock_webclipboard_impl.h"

#include <algorithm>

#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/renderer/clipboard_utils.h"
#include "third_party/WebKit/public/platform/WebCommon.h"
#include "third_party/WebKit/public/platform/WebDragData.h"
#include "third_party/WebKit/public/platform/WebImage.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/size.h"
#include "webkit/glue/webkit_glue.h"

using blink::WebDragData;
using blink::WebString;
using blink::WebURL;
using blink::WebVector;

namespace content {

MockWebClipboardImpl::MockWebClipboardImpl() {}

MockWebClipboardImpl::~MockWebClipboardImpl() {}

bool MockWebClipboardImpl::isFormatAvailable(Format format, Buffer buffer) {
  switch (format) {
    case FormatPlainText:
      return !m_plainText.isNull();

    case FormatHTML:
      return !m_htmlText.isNull();

    case FormatSmartPaste:
      return m_writeSmartPaste;

    default:
      NOTREACHED();
      return false;
  }

  switch (buffer) {
    case BufferStandard:
      break;
    case BufferSelection:
#if defined(OS_POSIX) && !defined(OS_MACOSX)
      break;
#endif
    default:
      NOTREACHED();
      return false;
  }

  return true;
}

WebVector<WebString> MockWebClipboardImpl::readAvailableTypes(
    Buffer buffer,
    bool* containsFilenames) {
  *containsFilenames = false;
  std::vector<WebString> results;
  if (!m_plainText.isEmpty()) {
    results.push_back(WebString("text/plain"));
  }
  if (!m_htmlText.isEmpty()) {
    results.push_back(WebString("text/html"));
  }
  if (!m_image.isNull()) {
    results.push_back(WebString("image/png"));
  }
  for (std::map<base::string16, base::string16>::const_iterator it =
           m_customData.begin();
       it != m_customData.end(); ++it) {
    CHECK(std::find(results.begin(), results.end(), it->first) ==
          results.end());
    results.push_back(it->first);
  }
  return results;
}

blink::WebString MockWebClipboardImpl::readPlainText(
    blink::WebClipboard::Buffer buffer) {
  return m_plainText;
}

// TODO(wtc): set output argument *url.
blink::WebString MockWebClipboardImpl::readHTML(
    blink::WebClipboard::Buffer buffer,
    blink::WebURL* url,
    unsigned* fragmentStart,
    unsigned* fragmentEnd) {
  *fragmentStart = 0;
  *fragmentEnd = static_cast<unsigned>(m_htmlText.length());
  return m_htmlText;
}

blink::WebData MockWebClipboardImpl::readImage(
    blink::WebClipboard::Buffer buffer) {
  std::string data;
  std::vector<unsigned char> encoded_image;
  // TODO(dcheng): Verify that we can assume the image is ARGB8888. Note that
  // for endianess reasons, it will be BGRA8888 on Windows.
  const SkBitmap& bitmap = m_image.getSkBitmap();
  SkAutoLockPixels lock(bitmap);
  gfx::PNGCodec::Encode(static_cast<unsigned char*>(bitmap.getPixels()),
#if defined(OS_ANDROID)
                        gfx::PNGCodec::FORMAT_RGBA,
#else
                        gfx::PNGCodec::FORMAT_BGRA,
#endif
                        gfx::Size(bitmap.width(), bitmap.height()),
                        bitmap.rowBytes(),
                        false /* discard_transparency */,
                        std::vector<gfx::PNGCodec::Comment>(),
                        &encoded_image);
  data.assign(reinterpret_cast<char*>(vector_as_array(&encoded_image)),
              encoded_image.size());
  return data;
}

blink::WebString MockWebClipboardImpl::readCustomData(
    blink::WebClipboard::Buffer buffer,
    const blink::WebString& type) {
  std::map<base::string16, base::string16>::const_iterator it =
      m_customData.find(type);
  if (it != m_customData.end())
    return it->second;
  return blink::WebString();
}

void MockWebClipboardImpl::writeHTML(const blink::WebString& htmlText,
                                     const blink::WebURL& url,
                                     const blink::WebString& plainText,
                                     bool writeSmartPaste) {
  clear();

  m_htmlText = htmlText;
  m_plainText = plainText;
  m_writeSmartPaste = writeSmartPaste;
}

void MockWebClipboardImpl::writePlainText(const blink::WebString& plain_text) {
  clear();

  m_plainText = plain_text;
}

void MockWebClipboardImpl::writeURL(const blink::WebURL& url,
                                    const blink::WebString& title) {
  clear();

  m_htmlText = WebString::fromUTF8(URLToMarkup(url, title));
  m_plainText = url.spec().utf16();
}

void MockWebClipboardImpl::writeImage(const blink::WebImage& image,
                                      const blink::WebURL& url,
                                      const blink::WebString& title) {
  if (!image.isNull()) {
    clear();

    m_plainText = m_htmlText;
    m_htmlText = WebString::fromUTF8(URLToImageMarkup(url, title));
    m_image = image;
  }
}

void MockWebClipboardImpl::writeDataObject(const WebDragData& data) {
  clear();

  const WebVector<WebDragData::Item>& itemList = data.items();
  for (size_t i = 0; i < itemList.size(); ++i) {
    const WebDragData::Item& item = itemList[i];
    switch (item.storageType) {
      case WebDragData::Item::StorageTypeString: {
        if (EqualsASCII(item.stringType, ui::Clipboard::kMimeTypeText)) {
          m_plainText = item.stringData;
          continue;
        }
        if (EqualsASCII(item.stringType, ui::Clipboard::kMimeTypeHTML)) {
          m_htmlText = item.stringData;
          continue;
        }
        m_customData.insert(std::make_pair(item.stringType, item.stringData));
        continue;
      }
      case WebDragData::Item::StorageTypeFilename:
      case WebDragData::Item::StorageTypeBinaryData:
        NOTREACHED();  // Currently unused by the clipboard implementation.
    }
  }
}

void MockWebClipboardImpl::clear() {
  m_plainText = WebString();
  m_htmlText = WebString();
  m_image.reset();
  m_customData.clear();
  m_writeSmartPaste = false;
}

}  // namespace content
