/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <xenia/kernel/fs/devices/disc_image_device.h>

#include <poly/math.h>
#include <xenia/kernel/fs/gdfx.h>
#include <xenia/kernel/fs/devices/disc_image_entry.h>


using namespace xe;
using namespace xe::kernel;
using namespace xe::kernel::fs;

DiscImageDevice::DiscImageDevice(const std::string& path,
                                 const std::wstring& local_path)
    : Device(path), local_path_(local_path), mmap_(nullptr), gdfx_(nullptr) {}

DiscImageDevice::~DiscImageDevice() {
  delete gdfx_;
  xe_mmap_release(mmap_);
}

int DiscImageDevice::Init() {
  mmap_ = xe_mmap_open(kXEFileModeRead, local_path_.c_str(), 0, 0);
  if (!mmap_) {
    XELOGE("Disc image could not be mapped");
    return 1;
  }

  gdfx_ = new GDFX(mmap_);
  GDFX::Error error = gdfx_->Load();
  if (error != GDFX::kSuccess) {
    XELOGE("GDFX init failed: %d", error);
    return 1;
  }

  //gdfx_->Dump();

  return 0;
}

Entry* DiscImageDevice::ResolvePath(const char* path) {
  // The filesystem will have stripped our prefix off already, so the path will
  // be in the form:
  // some\PATH.foo

  XELOGFS("DiscImageDevice::ResolvePath(%s)", path);

  GDFXEntry* gdfx_entry = gdfx_->root_entry();

  // Walk the path, one separator at a time.
  auto path_parts = poly::split_path(path);
  for (auto& part : path_parts) {
    gdfx_entry = gdfx_entry->GetChild(part.c_str());
    if (!gdfx_entry) {
      // Not found.
      return nullptr;
    }
  }

  Entry::Type type = gdfx_entry->attributes & X_FILE_ATTRIBUTE_DIRECTORY ?
      Entry::kTypeDirectory : Entry::kTypeFile;
  return new DiscImageEntry(
      type, this, path, mmap_, gdfx_entry);
}

X_STATUS DiscImageDevice::QueryVolume(XVolumeInfo* out_info, size_t length) {
  assert_always();
  return X_STATUS_NOT_IMPLEMENTED;
}

X_STATUS DiscImageDevice::QueryFileSystemAttributes(XFileSystemAttributeInfo* out_info, size_t length) {
  assert_always();
  return X_STATUS_NOT_IMPLEMENTED;
}
