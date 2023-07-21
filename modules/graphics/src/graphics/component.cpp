#include "graphics/graphics_new_include.h"
#include "util/data.h"

bool graphics::phenyl_from_data (const util::DataValue& dataVal, graphics::Model2D& comp) {
    if (!dataVal.is<std::string>()) {
        return false;
    }

    comp.modelName = dataVal.get<std::string>();

    return true;
}

util::DataValue graphics::phenyl_to_data (const graphics::Model2D& comp) {
    return (util::DataValue)comp.modelName;
}

/*bool graphics::phenyl_from_data (const util::DataValue& dataVal, graphics::GlobalTransform2D& comp) {
    if (!dataVal.is<util::DataObject>()) {
        return false;
    }

    auto& obj = dataVal.get<util::DataObject>();
    if (obj.contains("transform")) {
        auto& arr = obj.at("transform").get<util::DataArray>();
        for (int i = 0; i < 2; i++) {
            comp.transform[i] = arr[i].get<glm::vec2>();
        }
    }
    comp.rotTransform = comp.transform;

    return true;

   if (!dataVal.is<util::DataObject>()) {
       return false;
   }

   const auto& obj = dataVal.get<util::DataObject>();
   if (!obj.contains("transform")) {
       return false;
   }

    const auto& transformVal = obj.at("transform");

    return phenyl_from_data(transformVal, comp.transform2D);
}*/

/*util::DataValue graphics::phenyl_to_data (const graphics::GlobalTransform2D& comp) {
    util::DataObject obj;
    util::DataArray arr;
    for (int i = 0; i < 2; i++) {
        arr.emplace_back(comp.transform[i]);
    }
    obj["transform"] = std::move(arr);
    return (util::DataValue)obj;

    util::DataObject obj;
    obj["transform"] = comp.transform2D;

    return obj;
}*/