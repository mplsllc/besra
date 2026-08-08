#ifndef BESRA_QT6_RESOURCES_H
#define BESRA_QT6_RESOURCES_H

namespace besra {

/**
 * Load the embedded message catalogue into the core's Messages hash.
 * Call once at startup, before any UI that calls messages_get().
 */
void load_messages();

} // namespace besra

#endif
