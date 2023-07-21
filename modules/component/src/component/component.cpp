#include "component/component.h"

using namespace component;

detail::ComponentSet::ComponentSet (std::size_t startCapacity, std::size_t compSize) : ids{}, indexSet{}, data{std::make_unique<std::byte[]>(startCapacity * compSize)},
        compSize{compSize}, dataSize{0}, dataCapacity{startCapacity} {
    ids.reserve(startCapacity);
    indexSet.reserve(startCapacity);
}

detail::ComponentSet::~ComponentSet () = default;

void detail::ComponentSet::guaranteeEntityIndex (std::size_t index) {
    while (indexSet.size() < index) {
        indexSet.push_back(EMPTY_INDEX);
    }
}

std::byte* detail::ComponentSet::getComponentUntyped (EntityId id) const {
    assert(indexSet.size() > id.id - 1);

    auto compIndex = indexSet[id.id - 1];
    if (compIndex == EMPTY_INDEX) {
        return nullptr;
    } else {
        return data.get() + (compSize * compIndex);
    }
}

std::byte* detail::ComponentSet::tryInsert (EntityId id) {
    assert(indexSet.size() > id.id - 1);

    if (indexSet[id.id - 1] != EMPTY_INDEX) {
        return nullptr;
    }

    guaranteeCapacity(dataSize + 1);

    auto* ptr = data.get() + (compSize * dataSize);
    indexSet[id.id - 1] = dataSize;
    ids.push_back(id);

    dataSize++;
    return ptr;
}

void detail::ComponentSet::deleteComp (EntityId id) {
    assert(indexSet.size() > id.id - 1);
    if (indexSet[id.id - 1] == EMPTY_INDEX) {
        return;
    }

    auto compIndex = indexSet[id.id - 1];
    auto* compPtr = data.get() + compSize * compIndex;
    if (compIndex == dataSize - 1) {
        deleteTypedComp(compPtr);
    } else {
        auto oldCompPtr = data.get() + compSize * (dataSize - 1);
        moveTypedComp(compPtr, oldCompPtr);

        auto movedId = ids[dataSize - 1];
        ids[compIndex] = movedId;
        indexSet[movedId.id - 1] = compIndex;
    }

    indexSet[id.id - 1] = EMPTY_INDEX;
    ids.pop_back();
    dataSize--;
}

void detail::ComponentSet::guaranteeCapacity (std::size_t capacity) {
    if (dataCapacity >= capacity) {
        return;
    }

    while (dataCapacity < capacity) {
        dataCapacity *= RESIZE_FACTOR;
    }

    auto newData = std::make_unique<std::byte[]>(dataCapacity * compSize);
    moveAllComps(newData.get(), data.get(), dataSize);

    data = std::move(newData);
}

void detail::ComponentSet::clear () {
    for (std::size_t i = 0; i < dataSize; i++) {
        deleteTypedComp(data.get() + (i * compSize));
        auto index = ids[i].id - 1;

        indexSet[index] = EMPTY_INDEX;
        ids[i] = EntityId{};
    }
}

bool detail::ComponentSet::hasComp (EntityId id) const {
    assert(indexSet.size() > id.id - 1);
    return indexSet[id.id - 1] != EMPTY_INDEX;
}


detail::EntityIdList::EntityIdList (std::size_t capacity) : idSlots{}, freeListStart{FREE_LIST_EMPTY}, numEntities{0} {
    idSlots.reserve(capacity);
}

EntityId detail::EntityIdList::newId () {
    if (freeListStart == FREE_LIST_EMPTY) {
        if (idSlots.size() >= MAX_NUM_IDS) {
            component::logging::log(LEVEL_ERROR, "Too many entity ids!");
            return EntityId{};
        }
        idSlots.push_back(1);
        numEntities++;

        assert(idSlots.size() < (std::size_t{1} << FREE_LIST_BITS));

        return EntityId{1, static_cast<unsigned int>(idSlots.size())};
    } else {
        auto index = freeListStart - 1;

        assert(idSlots.size() > index);
        assert(EMPTY_BIT & idSlots.at(index));

        freeListStart = (idSlots.at(index) & FREE_LIST_MASK) >> GEN_BITS;
        auto gen = static_cast<unsigned int>(idSlots.at(index) & GEN_MASK);
        gen++;
        if (gen == 0) {
            gen = 1;
        }

        idSlots.at(index) = gen;
        numEntities++;

        return EntityId{gen, static_cast<unsigned int>(index + 1)};
    }
}

bool detail::EntityIdList::check (EntityId id) const {
    if (id.id == 0 || id.id > idSlots.size()) {
        return false;
    }

    return (idSlots[id.id - 1] & EMPTY_BIT) == 0 && idSlots[id.id - 1] == id.generation;
}

void detail::EntityIdList::removeId (EntityId id) {
    assert(id && id.id <= idSlots.size());
    assert(check(id));

    idSlots[id.id - 1] |= EMPTY_BIT | (freeListStart << GEN_BITS);
    freeListStart = id.id;
    numEntities--;
}

void detail::EntityIdList::clear () {
    for (std::size_t i = 0; i < idSlots.size(); i++) {
        if ((idSlots[i] & EMPTY_BIT) == 0) {
            idSlots[i] |= EMPTY_BIT | (freeListStart << GEN_BITS);
            freeListStart = i + 1;
        }
    }

    numEntities = 0;
}

std::size_t detail::EntityIdList::size () const {
    return numEntities;
}

std::size_t detail::EntityIdList::maxIndex () const {
    return idSlots.size();
}

detail::EntityIdList::iterator detail::EntityIdList::begin () const {
    return cbegin();
}

detail::EntityIdList::const_iterator detail::EntityIdList::cbegin () const {
    return const_iterator{this, 0};
}

detail::EntityIdList::iterator detail::EntityIdList::end () const {
    return cend();
}

detail::EntityIdList::const_iterator detail::EntityIdList::cend () const {
    return const_iterator{this, idSlots.size()};
}

detail::EntityIdList::IdIterator::IdIterator (const detail::EntityIdList* idList, std::size_t slotPos) : idList{idList}, slotPos{slotPos} {
    if (slotPos != idList->idSlots.size() && (idList->idSlots[slotPos] & EMPTY_BIT)) {
        next();
    }
}

void detail::EntityIdList::IdIterator::next () {
    if (slotPos == idList->idSlots.size()) {
        return;
    }

    while (++slotPos < idList->idSlots.size() && (idList->idSlots[slotPos] & EMPTY_BIT)) {}
}

void detail::EntityIdList::IdIterator::prev () {
    if (slotPos == 0) {
        return;
    }

    while (--slotPos > 0 && (idList->idSlots[slotPos] & EMPTY_BIT)) {}
}

detail::EntityIdList::IdIterator::value_type detail::EntityIdList::IdIterator::operator* () const {
    assert((idList->idSlots[slotPos] & EMPTY_BIT) == 0);
    return EntityId{static_cast<unsigned int>(idList->idSlots[slotPos]), static_cast<unsigned int>(slotPos + 1)};
}

detail::EntityIdList::IdIterator& detail::EntityIdList::IdIterator::operator++ () {
    next();
    return *this;
}

detail::EntityIdList::IdIterator detail::EntityIdList::IdIterator::operator++ (int) {
    auto copy = *this;
    ++*this;

    return copy;
}

detail::EntityIdList::IdIterator& detail::EntityIdList::IdIterator::operator-- () {
    prev();
    return *this;
}

detail::EntityIdList::IdIterator detail::EntityIdList::IdIterator::operator-- (int) {
    auto copy = *this;
    --*this;

    return copy;
}

bool detail::EntityIdList::IdIterator::operator== (const detail::EntityIdList::IdIterator& other) const {
    return idList == other.idList && slotPos == other.slotPos;
}
