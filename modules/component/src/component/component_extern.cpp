#include "component/component.h"
#include "component/component_serialiser.h"

#include "component/component_new.h"

using namespace component;
namespace component {
    template class ComponentManager<PHENYL_MAX_COMPONENTS>;
    template class ComponentView<PHENYL_MAX_COMPONENTS>;

    //template class ObjectSerialiser<PHENYL_MAX_COMPONENTS>;
    //template class ObjectComponentFactory<PHENYL_MAX_COMPONENTS>;
}

