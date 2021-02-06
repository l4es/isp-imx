/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "cam_impexinfo.hpp"
#include <ebase/types.h>
#include <fstream>
#include <sstream>

template <class T> class TagValue : public Tag {
public:
  typedef T value_type;
  typedef T &reference_type;
  typedef const T &const_reference_type;

public:
  static Tag::Type getType();
  static Tag *create(const std::string &id, const_reference_type value) {
    return new TagValue<T>(id, value);
  }

public:
  TagValue(const std::string &id, const_reference_type value)
      : Tag(TagValue<T>::getType(), id), value(value) {}

public:
  virtual std::string toString() const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
  virtual void fromString(const std::string &str) {
    std::stringstream iss;
    iss << str;
    iss >> value;
  }

  value_type get() const { return value; }
  void get(reference_type v) const { v = value; }
  void set(const_reference_type v) { value = v; }

private:
  value_type value;
};

// explicit template instantiation
template bool Tag::getValue<bool>(bool &) const;
template bool Tag::getValue<int32_t>(int32_t &) const;
template bool Tag::getValue<uint32_t>(uint32_t &) const;
template bool Tag::getValue<float>(float &) const;
template bool Tag::getValue<std::string>(std::string &) const;

template bool Tag::setValue<bool>(const bool &);
template bool Tag::setValue<int32_t>(const int32_t &);
template bool Tag::setValue<uint32_t>(const uint32_t &);
template bool Tag::setValue<float>(const float &);
template bool Tag::setValue<std::string>(const std::string &);

template void TagMap::insert<bool>(const bool &, const std::string &,
                                   const std::string &);
template void TagMap::insert<int32_t>(const int32_t &, const std::string &,
                                      const std::string &);
template void TagMap::insert<uint32_t>(const uint32_t &, const std::string &,
                                       const std::string &);
template void TagMap::insert<float>(const float &, const std::string &,
                                    const std::string &);
template void TagMap::insert<std::string>(const std::string &,
                                          const std::string &,
                                          const std::string &);

// template specialization
template <> Tag::Type TagValue<bool>::getType() { return Tag::TYPE_BOOL; }
template <> Tag::Type TagValue<int32_t>::getType() { return Tag::TYPE_INT; }
template <> Tag::Type TagValue<uint32_t>::getType() { return Tag::TYPE_UINT32; }
template <> Tag::Type TagValue<float>::getType() { return Tag::TYPE_FLOAT; }
template <> Tag::Type TagValue<std::string>::getType() {
  return Tag::TYPE_STRING;
}

template <> std::string TagValue<uint32_t>::toString() const {
  std::ostringstream oss;
  oss << "0x" << std::hex;
  oss << value;
  return oss.str();
}

/*
template<>
std::string
TagValue<bool>::toString() const
{
    return value ? "1" : "0";
}


template<>
void
TagValue<bool>::fromString( const std::string& str )
{
    value = ( 0 == str.compare( 1, 1, "0" ) ) ? false : true;
}
*/

template <class T> bool Tag::getValue(T &value) const {
  if (TagValue<T>::getType() == getType()) {
    static_cast<const TagValue<T> *>(this)->get(value);
    return true;
  }

  return false;
}

template <class T> bool Tag::setValue(const T &value) {
  if (TagValue<T>::getType() == getType()) {
    static_cast<TagValue<T> *>(this)->set(value);
    return true;
  }

  return false;
}

TagMap::TagMap() {}

TagMap::~TagMap() { clear(); }

void TagMap::clear() {
  for (categoryIterator ci = data.begin(); ci != data.end(); ++ci) {
    for (tagIterator ti = ci->second.begin(); ti != ci->second.end(); ++ti) {
      delete *ti;
    }
    ci->second.clear();
  }
  data.clear();
}

bool TagMap::containes(const std::string &id,
                       const std::string &category) const {
  Tag *t = tag(id, category);
  if (t) {
    return true;
  }

  return false;
}

template <class T>
void TagMap::insert(const T &value, const std::string &id,
                    const std::string &category) {
  Tag *t = tag(id, category);
  if (t) {
    delete t;
    t = TagValue<T>::create(id, value);
    return;
  }

  data[category].push_back(TagValue<T>::create(id, value));
}

void TagMap::remove(const std::string &id, const std::string &category) {
  categoryIterator ci = data.find(category);
  if (ci != data.end()) {
    for (tagIterator ti = ci->second.begin(); ti != ci->second.end(); ++ti) {
      if ((*ti)->getId() == id) {
        delete *ti;
        ci->second.erase(ti);
      }
    }

    if (ci->second.empty()) {
      data.erase(ci);
    }
  }
}

Tag *TagMap::tag(const std::string &id, const std::string &category) const {
  const_category_iterator ci = data.find(category);
  if (ci != data.end()) {
    for (const_tag_iterator ti = ci->second.begin(); ti != ci->second.end();
         ++ti) {
      if ((*ti)->getId() == id) {
        return *ti;
      }
    }
  }

  return NULL;
}

TagMap::const_category_iterator TagMap::begin() const { return data.begin(); }

TagMap::const_category_iterator TagMap::end() const { return data.end(); }

TagMap::const_tag_iterator TagMap::begin(const_category_iterator iter) const {
  return iter->second.begin();
}

TagMap::const_tag_iterator TagMap::end(const_category_iterator iter) const {
  return iter->second.end();
}

ImageExportInfo::ImageExportInfo(std::string filename) : filename(filename) {}

ImageExportInfo::~ImageExportInfo() {}

void ImageExportInfo::write() const {
  std::string filename2 = filename;

  size_t founddot = filename2.rfind('.');
  size_t foundslash = filename2.rfind('/');
  if (((founddot + 1) != std::string::npos) && (founddot > foundslash)) {
    filename2.replace(founddot + 1, 3, "cfg");
  } else {
    filename2.append(".cfg");
  }

  std::ofstream file;
  file.open(filename2.c_str(), std::ios::out | std::ios::trunc);

  for (TagMap::const_category_iterator ci = TagMap::begin();
       ci != TagMap::end(); ++ci) {
    file << "[" << ci->first << "]" << std::endl;
    for (TagMap::const_tag_iterator ti = TagMap::begin(ci);
         ti != TagMap::end(ci); ++ti) {
      file << (*ti)->getId() << "=" << (*ti)->toString() << std::endl;
    }
    file << std::endl;
  }

  file.close();
}
