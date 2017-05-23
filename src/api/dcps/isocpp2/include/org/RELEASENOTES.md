Release Notes                                              {#release_notes}
====================

[TOC]

Available since 6.6.0                                                                              {#release_notes_1}
=====================
- Added new ISO C++ DCPS API, called isocpp2.


Known Issues and Currently Unsupported Features                                                                    {#release_notes_known_issues}
=====================================

Although the ISO C++ DCPS API implements the majority of commonly used features, some functionality
of the OMG DDS specification and DDS-PSM-Cxx is not currently supported. Below is a list of known
issues and features that are not currently supported:

- [Find and Discovery functions](#release_notes_us_find_discovery)
- [OpenSplice API extensions](#release_notes_us_extensions)
- [Other known issues] (#release_notes_other_known_issues)


Find and Discovery functions                                                {#release_notes_us_find_discovery}
-------------------------
The following discovery functions are not currently supported:

- [dds/domain/discovery.hpp](@ref dds/domain/discovery.hpp)
- [dds/topic/discovery.hpp](@ref dds/topic/discovery.hpp)

OpenSplice API extensions                                                                        {#release_notes_us_extensions}
-------------------------
The implementation contains only API features that are present in the OMG DCPS specification at this time.
Other OpenSplice language bindings contain extensions to the specification such as additional QoS,
Entity operations, and Entities. These features are not yet available.

Other Known Issues                                                                        {#release_notes_other_known_issues}
------------------

- dds::topic::MultiTopic
    - MultiTopics are not currently supported.

- Release/Debug DLL mismatch issue
    - In order to build applications in DEBUG mode on Windows the ISO C++ library must match, this can be done by recompiling the custom lib as documented @ref isocpp2_customlibs "here"
