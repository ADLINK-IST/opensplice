#ifndef OMG_DDS_PUB_DETAIL_SAMPLE_HPP_
#define OMG_DDS_PUB_DETAIL_SAMPLE_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @todo RTF Issue - removed #include <dds/sub/TSample.hpp>
 */
#include <foo/bar/sub/Sample.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
/** @todo RTF Issue - This isn't right. There is no class TSample and if there was
 * it would presumably be templated on the data type. Pretty sure this needs to be
 * template <typename T> class Sample : foo::bar::sub::Sample<T> { };
 * ... or something like that?
 */
typedef dds::sub::TSample<foo::bar::sub::Sample> Sample;
}
}
}

#endif /* OMG_DDS_PUB_DETAIL_SAMPLE_HPP_ */
