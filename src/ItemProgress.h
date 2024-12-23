/**
 * @file ItemProgress.h
 * @brief This file contains the declaration of the ItemProgress class.
 */

#ifndef ItemProgress_H
#define ItemProgress_H

#include "MenuItem.h"

/**
 * @class ItemProgress
 * @brief ItemProgress indicates that the current item is a list.
 */
class ItemProgress : public MenuItem
{
private:
    fptrMapping mapping = NULL;   ///< Pointer to a mapping function
    fptrInt callback = NULL;      ///< Pointer to a callback function
    uint16_t progress = 0;        ///< The progress
    uint16_t initialProgress = 0; ///< Progress before edit started.
    uint8_t stepLength = 1;

public:
    /**
     * @brief Constructs a new ItemProgress object.
     *
     * @param key The key of the menu item.
     * @param start The starting position of the list.
     * @param mapping A pointer to the mapping function.
     * @param callback A pointer to the callback function to execute when this
     * menu item is selected.
     */
    ItemProgress(const char *key, uint16_t start, uint8_t stepLength,
                 fptrMapping mapping, fptrInt callback)
        : MenuItem(key, MENU_ITEM_PROGRESS),
          mapping(mapping),
          callback(callback),
          progress(start),
          initialProgress(start),
          stepLength(stepLength) {}

    ItemProgress(const char *key, uint16_t start, fptrInt callback)
        : ItemProgress(key, start, 1, NULL, callback) {}

    ItemProgress(const char *key, fptrInt callback)
        : ItemProgress(key, 0, 1, NULL, callback) {}

    ItemProgress(const char *key, uint8_t stepLength, fptrMapping mapping,
                 fptrInt callback)
        : ItemProgress(key, 0, stepLength, mapping, callback) {}

    /**
     * @brief Increments the progress of the list.
     */
    void increment() override
    {
        if (progress >= MAX_PROGRESS)
            return;
        progress += stepLength;
    }

    /**
     * @brief Decrements the progress of the list.
     */
    void decrement() override
    {
        if (progress <= MIN_PROGRESS)
            return;
        progress -= stepLength;
    }

    /**
     * Return the progress
     */
    uint16_t getItemIndex() override
    {
        return progress;
    }

    /**
     * Return the callback
     */
    fptrInt getCallbackInt() override { return callback; }

    /**
     * @brief Returns the value to be displayed.
     *        If there's no mapping, it returns the progress
     *        else it returns the mapped progress
     *
     * @return A pointer to the value of the current list item.
     */
    char *getValue() override
    {
        if (mapping == NULL)
        {
            static char buf[6];
            itoa(progress, buf, 10);
            return buf;
        }
        else
        {
            return mapping(progress);
        }
    }

    void setProgress(uint16_t p) { progress = p; }

    void saveProgress()
    {
        initialProgress = progress;
    }

    void restoreProgress()
    {
        progress = initialProgress;
    }
};

#define ITEM_PROGRESS(...) (new ItemProgress(__VA_ARGS__))

#endif // ItemProgress_H