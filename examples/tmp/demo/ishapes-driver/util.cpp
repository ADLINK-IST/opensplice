#include "util.hpp"

std::ostream&
operator << (std::ostream& os, const org::opensplice::demo::ShapeType& s) {
  os << "("  << s.color.in() << ","
     << s.x << ", " << s.y
     << ", " << s.shapesize << ")";
  return os;
}

std::ostream&
operator << (std::ostream& os, const dds::sub::SampleInfo& si) {
  os << "SampleInfo {"
     // << "\n\tDataState = " << si.state()
     << "\n\tvalid_data = " << si.valid()
     << "\n\tsource_timestamp = " << " timestamp... "//si.timestamp()
     << "\n\tinstance_handle = " << si.instance_handle()
     << "\n\tpublication_handle = " << si.publication_handle()
     // << "\n\trank = " << si.rank()
     << "}";

  return os;
}

void demo::ishapes::printShapeSample(const dds::sub::Sample<org::opensplice::demo::ShapeType>& s) {
   std::cout << s.data() << std::endl;
}

void demo::ishapes::printShape(const org::opensplice::demo::ShapeType& s) {
  std::cout << s << std::endl;
}
