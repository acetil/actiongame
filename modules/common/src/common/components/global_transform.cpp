#include "common/serializer_impl.h"

#include "common/components/2d/global_transform.h"

namespace phenyl::common {
    PHENYL_SERIALIZABLE(GlobalTransform2D, PHENYL_SERIALIZABLE_MEMBER(transform2D))
}