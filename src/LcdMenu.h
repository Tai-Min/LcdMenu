/*
  LcdMenu.h - Main include file for the LcdMenu Library

  MIT License

  Copyright (c) 2020-2023 Forntoh Thomas

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#pragma once

#ifndef USE_STANDARD_LCD
#include <LiquidCrystal_I2C.h>
#else
#include <LiquidCrystal.h>
#endif

#include <MenuItem.h>
#include <utils.h>

/**
 * The LcdMenu class contains all fields and methods to manipulate the menu
 * items.
 */
class LcdMenu
{
private:
    /**
     * ## Private Fields
     */

    /**
     * Cursor position
     */
    uint8_t cursorPosition = 1;
    /**
     * First visible item's position in the menu array
     */
    uint8_t top = 1;
    /**
     * Edit mode
     */
    bool isEditModeEnabled = false;
    /**
     * Will prevent left and right movement when in edit mode and character
     * picker is active
     */
    bool isCharPickerActive = false;
    /**
     * Last visible item's position in the menu array
     */
    uint8_t bottom = 0;
    /**
     * Rows on the LCD Display
     */
    uint8_t maxRows;
    /**
     * Columns on the LCD Display
     */
    uint8_t maxCols;
    /**
     * Column location of Blinker
     */
    uint8_t blinkerPosition = 0;
    /**
     * Array of menu items
     */
    MenuItem **currentMenuTable = NULL;
    /**
     * Number of menu items in current menu
     */
    size_t currentMenuSize = 0;
    /**
     * Down arrow (↓)
     */
    byte downArrow[8] = {
        0b00100, //   *
        0b00100, //   *
        0b00100, //   *
        0b00100, //   *
        0b00100, //   *
        0b10101, // * * *
        0b01110, //  ***
        0b00100  //   *
    };
    /**
     * Up arrow (↑)
     */
    byte upArrow[8] = {
        0b00100, //   *
        0b01110, //  ***
        0b10101, // * * *
        0b00100, //   *
        0b00100, //   *
        0b00100, //   *
        0b00100, //   *
        0b00100  //   *
    };
    /**
     * Cursor icon. Defaults to right arrow (→).
     */
    uint8_t cursorIcon = 0x7E;
    /**
     * Edit mode cursor icon. Defaults to left arrow (←).
     */
    uint8_t editCursorIcon = 0x7F;
    /**
     * Determines whether the screen should be updated after an action. Set it
     * to `false` when you want to display any other content on the screen then
     * set it back to `true` to show the menu.
     */
    bool enableUpdate = true;
    /**
     * The backlight state of the lcd
     */
    uint8_t backlightState = HIGH;

    /**
     * ## Private Methods
     */

    /**
     * Check if items all above the cursor are hidden.
     * Header is ignored in this check.
     * @param cursor cursor to use for check
     * @return 'bool' - true if all items above cursor are hidden
     * in current menu / submenu
     */
    bool checkAllAboveHidden(uint8_t cursor)
    {
        for (size_t i = cursor - 1; i > 0; i--)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if items all below the cursor are hidden.
     * Footer is ignored in this check.
     * @param cursor cursor to use for check
     * @return 'bool' - true if all items below cursor are hidden
     * in current menu / submenu
     */
    bool checkAllBelowHidden(uint8_t cursor)
    {
        for (size_t i = cursor + 1; i < currentMenuSize - 2; i++)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Count non hidden items above given cursor.
     * @param cursor cursor to use for check
     * @return 'uint8_t' - Number of non hidden items above cursor
     * in current menu / submenu
     */
    uint8_t countNonHiddenAbove(uint8_t cursor)
    {
        uint8_t res = 0;
        for (size_t i = cursor - 1; i > 0; i--)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                res++;
            }
        }
        return res;
    }

    /**
     * Count non hidden items below given cursor.
     * @param cursor cursor to use for check
     * @return 'uint8_t' - Number of non hidden items below cursor
     * in current menu / submenu
     */
    uint8_t countNonHiddenBelow(uint8_t cursor)
    {
        uint8_t res = 0;
        for (size_t i = cursor + 1; i < currentMenuSize - 2; i++)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                res++;
            }
        }
        return res;
    }

    /**
     * Count non hidden items.
     * @return 'uint8_t' - Number of non hidden items in current menu / submenu
     */
    uint8_t countNonHiddenItems()
    {
        size_t idx = 1;
        size_t res = 0;
        while (currentMenuTable[idx]->getType() != MENU_ITEM_END_OF_MENU)
        {
            if (!currentMenuTable[idx++]->isHidden())
            {
                res++;
            }
        }
        return res;
    }

    /**
     * Draws the cursor
     */
    void drawCursor()
    {
        //
        // Erases current cursor
        //
        for (uint8_t x = 0; x < maxRows; x++)
        {
            lcd->setCursor(0, x);
            lcd->print(" ");
        }
        //
        // draws a new cursor at [line]
        //
        uint8_t line = constrain(cursorPosition - top, 0, maxRows - 1);

        // TODO: for LCDs with more rows than 2?
        if (checkAllAboveHidden(cursorPosition))
        {
            line = 0;
        }

        lcd->setCursor(0, line);
        lcd->write(isEditModeEnabled ? editCursorIcon : cursorIcon);
#ifdef ItemInput_H
        //
        // If cursor is at MENU_ITEM_INPUT enable blinking
        //
        MenuItem *item = currentMenuTable[cursorPosition];
        if (item->getType() == MENU_ITEM_INPUT)
        {
            resetBlinker();
            if (isEditModeEnabled)
            {
                lcd->blink();
                return;
            }
        }
#endif
        lcd->noBlink();
    }
    /**
     * Draw the menu items with up and down indicators
     */
    void drawMenu()
    {
        lcd->clear();
        //
        // print the menu items
        //
        uint8_t t = top;
        uint8_t firstDrawnItemIdx = 255;
        for (uint8_t i = top; i <= bottom; i++)
        {
            MenuItem *item = currentMenuTable[t];

            while (item->isHidden())
            {
                t++;
                item = currentMenuTable[t];
            }

            if (firstDrawnItemIdx == 255)
            {
                firstDrawnItemIdx = t;
            }

            lcd->setCursor(1, map(i, top, bottom, 0, maxRows - 1));
            if (currentMenuTable[t]->getType() != MENU_ITEM_END_OF_MENU)
            {
                lcd->print(item->getText());
            }
            //
            // determine the type of item
            //
            switch (item->getType())
            {
#ifdef ItemToggle_H
            case MENU_ITEM_TOGGLE:
                //
                // append textOn or textOff depending on the state
                //
                lcd->print(":");
                lcd->print(item->isOn() ? item->getTextOn()
                                        : item->getTextOff());
                break;
#endif
#if defined(ItemProgress_H) || defined(ItemInput_H)
            case MENU_ITEM_INPUT:
            case MENU_ITEM_PROGRESS:
                //
                // append the value of the input
                //
                static char *buf = new char[maxCols];
                substring(item->getValue(), 0,
                          maxCols - strlen(item->getText()) - 2, buf);
                lcd->print(":");
                lcd->print(buf);
                break;
#endif
#ifdef ItemList_H
            case MENU_ITEM_LIST:
                //
                // append the value of the item at current list position
                //
                lcd->print(":");
                lcd->print(item->getItems()[item->getItemIndex()].substring(
                    0, maxCols - strlen(item->getText()) - 2));
                break;
#endif
            default:
                break;
            }
            // if we reached the end of menu, stop
            if (currentMenuTable[t]->getType() == MENU_ITEM_END_OF_MENU)
                break;

            t++;
        }

        if (isEditModeEnabled)
        {
            return;
        }

        // All entries fit the LCD so no arrows needed
        uint8_t nonHidden = countNonHiddenItems();
        if (nonHidden <= maxRows)
        {
            return;
        }

        uint8_t cursorLine = constrain(cursorPosition - top, 0, maxRows - 1);

        // TODO: for LCDs with more rows than 2?
        if (checkAllAboveHidden(cursorPosition) && cursorLine)
        {
            cursorLine = 0;
        }

        // Print up arrow
        if ((cursorLine == 0 && !checkAllAboveHidden(firstDrawnItemIdx) && cursorPosition > 1) ||
            (cursorLine != 0 && countNonHiddenAbove(firstDrawnItemIdx)))
        {
            lcd->setCursor(maxCols - 1, 0);
            lcd->write(byte(0));
        }

        // Print down arrow
        uint8_t lastDrawnItemIdx = t - 1;
        if ((countNonHiddenBelow(lastDrawnItemIdx)))
        {
            lcd->setCursor(maxCols - 1, maxRows - 1);
            lcd->write(byte(1));
        }
    }

    /**
     * Check if the cursor is at the start of the menu items
     * @return true : `bool` if it is at the start
     */
    bool isAtTheStart()
    {
        for (size_t i = cursorPosition - 1; i >= 0; i--)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                if (i == 0)
                {
                    return true;
                }
                return false;
            }
        }
        return true;
    }

    /**
     * Check if the cursor is at the end of the menu items
     * @return true : `bool` if it is at the end
     */
    bool isAtTheEnd()
    {
        for (size_t i = cursorPosition + 1; i < currentMenuSize; i++)
        {
            if (!currentMenuTable[i]->isHidden())
            {
                if (i == currentMenuSize - 1)
                {
                    return true;
                }
                return false;
            }
        }
        return true;
    }

    void enterSubMenu(MenuItem *item)
    {
        if (item->getSubMenu() == NULL)
            return;

        currentMenuTable[0]->setTop(top);
        currentMenuTable[0]->setBottom(bottom);
        currentMenuTable[0]->setCursorPosition(cursorPosition);

        top = 1;
        bottom = maxRows;
        cursorPosition = 1;

        currentMenuTable = item->getSubMenu();
        currentMenuSize = getMenuSize(currentMenuTable);

        update();
    }

    void leaveSubMenu(MenuItem *item)
    {
        if (item->getSubMenu() == NULL)
            return;

        currentMenuTable = item->getSubMenu();
        currentMenuSize = getMenuSize(currentMenuTable);

        top = currentMenuTable[0]->getTop();
        bottom = currentMenuTable[0]->getBottom();
        cursorPosition = currentMenuTable[0]->getCursorPosition();

        update();
    }

#ifdef ItemInput_H
    /**
     * Calculate and set the new blinker position
     */
    void resetBlinker()
    {
        //
        // calculate lower and upper bound
        //
        uint8_t lb = strlen(currentMenuTable[cursorPosition]->getText()) + 2;
        uint8_t ub = lb + strlen(currentMenuTable[cursorPosition]->getValue());
        ub = constrain(ub, lb, maxCols - 2);
        //
        // set cursor position
        //
        blinkerPosition = constrain(blinkerPosition, lb, ub);
        lcd->setCursor(blinkerPosition, cursorPosition - top);
    }
#endif

public:
    /**
     * ## Public Fields
     */

    /**
     * Time when the timer started in milliseconds
     */
    unsigned long startTime = 0;
    /**
     * How long should the display stay on
     */
    uint16_t timeout = 10000;
    /**
     * LCD Display
     */
#ifndef USE_STANDARD_LCD
    LiquidCrystal_I2C *lcd = NULL;
#else
    LiquidCrystal *lcd = NULL;
#endif

    /**
     * # Constructor
     */

    /**
     * Constructor for the LcdMenu class
     * @param maxRows rows on lcd display e.g. 4
     * @param maxCols columns on lcd display e.g. 20
     * @return new `LcdMenu` object
     */
    LcdMenu(uint8_t maxRows, uint8_t maxCols)
        : bottom(maxRows), maxRows(maxRows), maxCols(maxCols) {}

    /**
     * ## Public Methods
     */

    /**
     * Call this function in `setup()` to initialize the LCD and the custom
     * characters used as up and down arrows
     * @param lcd_Addr address of the LCD on the I2C bus (default 0x27)
     * @param menu menu to display
     */
    void setupLcdWithMenu(
#ifndef USE_STANDARD_LCD
        uint8_t lcd_Addr, MenuItem **menu)
    {
        lcd = new LiquidCrystal_I2C(lcd_Addr, maxCols, maxRows);
        lcd->init();
        lcd->backlight();
#else
        uint8_t rs, uint8_t en, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        MenuItem **menu)
    {
        this->lcd = new LiquidCrystal(rs, en, d0, d1, d2, d3);
        this->lcd->begin(maxCols, maxRows);
#endif
        lcd->clear();
        lcd->createChar(0, upArrow);
        lcd->createChar(1, downArrow);
        this->currentMenuTable = menu;
        this->currentMenuSize = getMenuSize(currentMenuTable);
        this->startTime = millis();
        update();
    }

    void setupLcdWithMenu(
#ifndef USE_STANDARD_LCD
        uint8_t lcd_Addr, MenuItem **menu, uint16_t timeout)
    {
        this->setupLcdWithMenu(lcd_Addr, menu);
#else
        uint8_t rs, uint8_t en, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        MenuItem **menu, uint16_t timeout)
    {
        this->setupLcdWithMenu(rs, en, d0, d1, d2, d3, menu);
#endif
        this->timeout = timeout;
    }
    /*
     * Draw the menu items and cursor
     */
    void update()
    {
        if (!enableUpdate)
            return;
        lcd->display();
        lcd->setBacklight(backlightState);
        drawMenu();
        drawCursor();
        startTime = millis();
    }

    /**
     * Execute an "up press" on menu
     * When edit mode is enabled, this action is skipped
     * @return 'bool' True if up action performed
     */
    bool up()
    {
        uint8_t cursorLine = constrain(cursorPosition - top, 0, maxRows - 1);
        if (checkAllAboveHidden(cursorPosition))
        {
            cursorLine = 0;
        }
        bool wasAtTop = cursorLine == 0;

        int8_t numSkipped = 1;
        do
        {
            if (isAtTheStart() || isEditModeEnabled)
            {
                return false;
            }
            cursorPosition--;
            numSkipped++;
        } while (currentMenuTable[cursorPosition]->isHidden());

        if (cursorPosition < top)
        {
            if (wasAtTop)
            {
                top = cursorPosition;
                bottom = top + maxRows - 1;
            }
        }

        update();
        return true;
    }
    /**
     * Execute a "down press" on menu
     * When edit mode is enabled, this action is skipped
     * @return 'bool' True if down action performed
     */
    bool down()
    {

        uint8_t cursorLine = constrain(cursorPosition - top, 0, maxRows - 1);
        if (checkAllBelowHidden(cursorPosition))
        {
            cursorLine = maxRows - 1;
        }
        bool wasAtBottom = cursorLine == maxRows - 1;

        int8_t numSkipped = -1;
        do
        {
            if (isAtTheEnd() || isEditModeEnabled)
            {
                return false;
            }
            cursorPosition++;
            numSkipped++;
        } while (currentMenuTable[cursorPosition]->isHidden());

        if (cursorPosition > bottom)
        {
            if (wasAtBottom)
            {
                top = cursorPosition - numSkipped - 1;
                bottom = top + maxRows - 1;
            }
        }

        update();
        return true;
    }

    /**
     * Execute an "enter" action on menu.
     *
     * It does the following depending on the type of the current menu item:
     *
     * - Open a sub menu.
     * - Execute a callback action.
     * - Toggle the state of an item.
     */
    void enter()
    {
        size_t pos = cursorPosition;
        MenuItem *item = currentMenuTable[pos];

        //
        // determine the type of menu entry, then execute it
        //
        switch (item->getType())
        {
        //
        // switch the menu to the selected sub menu
        //
        case MENU_ITEM_SUB_MENU:
        {
            enterSubMenu(item);
            break;
        }
#ifdef ItemCommand_H
        //
        // execute the menu item's function
        //
        case MENU_ITEM_COMMAND:
        {
            //
            // execute the menu item's function
            //
            if (item->getCallback() != NULL)
                (item->getCallback())();
            //
            // display the menu again
            //
            update();
            break;
        }
#endif
#ifdef ItemToggle_H
        case MENU_ITEM_TOGGLE:
        {
            //
            // toggle the value of isOn
            //
            item->setIsOn(!item->isOn());
            //
            // execute the menu item's function
            //
            if (item->getCallbackInt() != NULL)
                (item->getCallbackInt())(item->isOn());
            //
            // display the menu again
            //
            update();
            break;
        }
#endif
#ifdef ItemInput_H
        case MENU_ITEM_INPUT:
        {
            //
            // enter editmode
            //
            if (!isInEditMode())
            {
                isEditModeEnabled = true;
                // blinker will be drawn
                drawCursor();
            }
            break;
        }
#endif
        case MENU_ITEM_PROGRESS:
        case MENU_ITEM_LIST:
        {
            //
            // execute the menu item's function
            //
            if (!isInEditMode())
            {
                isEditModeEnabled = true;
                item->saveProgress();

                drawCursor();
            }
            break;
        }
        }
    }
    /**
     * Execute a "backpress" action on menu.
     *
     * Navigates up once.
     */
    void back(bool editCancelled = false)
    {
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        // Back action different when on ItemInput
        //
        if (isInEditMode())
        {
            switch (item->getType())
            {
#ifdef ItemInput_H
            case MENU_ITEM_INPUT:
                // Disable edit mode
                isEditModeEnabled = false;
                update();
                // Execute callback function
                if (item->getCallbackStr() != NULL)
                    (item->getCallbackStr())(item->getValue());
                // Interrupt going back to parent menu
                return;
#endif
#if defined(ItemProgress_H) || defined(ItemList_H)
            case MENU_ITEM_LIST:
            case MENU_ITEM_PROGRESS:
                // Disable edit mode
                isEditModeEnabled = false;

                if (editCancelled)
                {
                    item->restoreProgress();
                }

                // Execute callback function
                if (item->getCallbackInt() != NULL)
                    (item->getCallbackInt())(item->getItemIndex());
                // Interrupt going back to parent menu

                update();
                return;
#endif
            default:
                break;
            }
        }
        //
        // check if this is a sub menu, if so go back to its parent
        //
        if (isSubMenu())
        {
            leaveSubMenu(currentMenuTable[0]);
        }
    }
    /**
     * Execute a "left press" on menu
     *
     * *NB: Works only for `ItemInput` and `ItemList` types*
     *
     * Moves the cursor one step to the left.
     */
    void left()
    {
        //
        if (isInEditMode() && isCharPickerActive)
            return;
        //
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        // get the type of the currently displayed menu
        //
#ifdef ItemList_H
        uint8_t previousIndex = item->getItemIndex();
#endif
        switch (item->getType())
        {
#ifdef ItemList_H
        case MENU_ITEM_LIST:
        {
            item->setItemIndex(item->getItemIndex() - 1);
            if (previousIndex != item->getItemIndex())
                update();
            break;
        }
#endif
#ifdef ItemInput_H
        case MENU_ITEM_INPUT:
        {
            blinkerPosition--;
            resetBlinker();
            break;
        }
#endif
#ifdef ItemProgress_H
        case MENU_ITEM_PROGRESS:
        {
            if (isInEditMode())
            {
                item->decrement();
                update();
            }
        }
#endif
        }
    }
    /**
     * Execute a "right press" on menu
     *
     * *NB: Works only for `ItemInput` and `ItemList` types*
     *
     * Moves the cursor one step to the right.
     */
    void right()
    {
        //
        // Is the menu in edit mode and is the character picker active?
        //
        if (isInEditMode() && isCharPickerActive)
            return;
        //
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        // get the type of the currently displayed menu
        //
        switch (item->getType())
        {
#ifdef ItemList_H
        case MENU_ITEM_LIST:
        {
            item->setItemIndex((item->getItemIndex() + 1) %
                               item->getItemCount());
            // constrain(item->itemIndex + 1, 0, item->itemCount - 1);
            update();
            break;
        }
#endif
#ifdef ItemInput_H
        case MENU_ITEM_INPUT:
        {
            blinkerPosition++;
            resetBlinker();
            break;
        }
#endif
#ifdef ItemProgress_H
        case MENU_ITEM_PROGRESS:
        {
            if (isInEditMode())
            {
                item->increment();
                update();
            }
            break;
        }
#endif
        }
    }
#ifdef ItemInput_H
    /**
     * Execute a "backspace cmd" on menu
     *
     * *NB: Works only for `ItemInput` type*
     *
     * Removes the character at the current cursor position.
     */
    void backspace()
    {
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        if (item->getType() != MENU_ITEM_INPUT)
            return;
        //
        uint8_t p = blinkerPosition - (strlen(item->getText()) + 2) - 1;
        remove(item->getValue(), p, 1);

        blinkerPosition--;
        update();
    }
    /**
     * Display text at the cursor position
     * used for `Input` type menu items
     * @param character character to append
     */
    void type(char character)
    {
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        if (item->getType() != MENU_ITEM_INPUT || !isEditModeEnabled)
            return;
        //
        // calculate lower and upper bound
        //
        uint8_t length = strlen(item->getValue());
        uint8_t lb = strlen(item->getText()) + 2;
        uint8_t ub = lb + length;
        ub = constrain(ub, lb, maxCols - 2);
        //
        // update text
        //
        if (blinkerPosition < ub)
        {
            static char start[10];
            static char end[10];
            static char *joined = new char[maxCols - lb];
            substring(item->getValue(), 0, blinkerPosition - lb, start);
            substring(item->getValue(), blinkerPosition + 1 - lb, length, end);
            concat(start, character, end, joined);
            item->setValue(joined);
        }
        else
        {
            static char *buf = new char[length + 2];
            concat(item->getValue(), character, buf);
            item->setValue(buf);
        }
        //
        isCharPickerActive = false;
        //
        // update blinker position
        //
        blinkerPosition++;
        //
        // repaint menu
        //
        update();
    }
    /**
     * Draw a character on the display
     * used for `Input` type menu items.
     * @param c character to draw
     */
    void drawChar(char c)
    {
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        if (item->getType() != MENU_ITEM_INPUT || !isEditModeEnabled)
            return;
        //
        // draw the character without updating the menu item
        //
        uint8_t line = constrain(cursorPosition - top, 0, maxRows - 1);
        lcd->setCursor(blinkerPosition, line);
        lcd->print(c);
        resetBlinker();
        //
        isCharPickerActive = true;
    }
    /**
     * Clear the value of the input field
     */
    void clear()
    {
        MenuItem *item = currentMenuTable[cursorPosition];
        //
        if (item->getType() != MENU_ITEM_INPUT)
            return;
        //
        // set the value
        //
        item->setValue((char *)"");
        //
        // update blinker position
        //
        blinkerPosition = 0;
        //
        // repaint menu
        //
        update();
    }
#endif
    /**
     * Set the character used to visualize the cursor.
     * @param newIcon character to use for default cursor
     * @param newEditIcon character use for edit mode cursor
     */
    void setCursorIcon(uint8_t newIcon, uint8_t newEditIcon)
    {
        cursorIcon = newIcon;
        editCursorIcon = newEditIcon;
        drawCursor();
    }
    /**
     * When you want to display any other content on the screen then
     * call this function then display your content, later call
     * `show()` to show the menu
     */
    void hide()
    {
        enableUpdate = false;
        lcd->clear();
    }
    /**
     * Show the menu
     */
    void show()
    {
        enableUpdate = true;
        update();
    }
    /**
     * To know weather the menu is in edit mode or not
     * @return `bool` - isEditModeEnabled
     */
    bool isInEditMode() { return isEditModeEnabled; }
    /**
     * Get the current cursor position
     * @return `cursorPosition` e.g. 1, 2, 3...
     */
    uint8_t getCursorPosition() { return this->cursorPosition; }
    /**
     * Set the current cursor position
     * @param position
     */
    void setCursorPosition(uint8_t position)
    {
        this->cursorPosition = position;
    }
    /**
     * Update timer and turn off display on timeout
     */
    void updateTimer()
    {
        if (millis() == startTime + timeout)
        {
            lcd->noDisplay();
            lcd->noBacklight();
        }
    }
    /**
     * Check if currently displayed menu is a sub menu.
     */
    bool isSubMenu()
    {
        byte menuItemType = currentMenuTable[0]->getType();
        return menuItemType == MENU_ITEM_SUB_MENU_HEADER;
    }

    size_t getMenuSize(MenuItem **menu)
    {
        size_t s = 0;

        while (menu[s]->getType() != MENU_ITEM_END_OF_MENU)
        {
            s++;
        }
        s++;
        return s;
    }

    /**
     * Get a `MenuItem` at position
     * @return `MenuItem` - item at `position`
     */
    MenuItem *getItemAt(uint8_t position) { return currentMenuTable[position]; }
    /**
     * Get a `MenuItem` at position using operator function
     * e.g `menu[menu.getCursorPosition()]` will return the item at the
     * current cursor position NB: This is relative positioning (i.e. if a
     * submenu is currently being displayed, menu[1] will return item 1 in
     * the current menu)
     * @return `MenuItem` - item at `position`
     */
    MenuItem *operator[](const uint8_t position)
    {
        return currentMenuTable[position];
    }
    /**
     * Set the Backlight state
     * @param state
     */
    void setBacklight(uint8_t state)
    {
        backlightState = state;
        update();
    }
};
