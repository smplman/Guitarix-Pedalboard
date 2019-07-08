
/* -*- C++ -*- */
#pragma once
// a full automated Serial file select
// plugin for arduino menu library
// requires a dynamic menu (MENU_USERAM)
// IO: Serial
// July 2019 - Stephen Peery [smp4488@gmail.com]
#include <menu.h>
#include "SerialFileListing.h"

using namespace Menu;

template <typename SDC>
class FSO
{
public:
    using Type = SDC;
    Type &sdc;

    idx_t maxSz = 32;
    long cacheStart = 0;
    String cache[32];
    long size = 0;

    FSO(Type &sdc) : sdc(sdc) {}
    virtual ~FSO(){}

    void refresh(long start=0)
    {
        if (start < 0) start=0;
        cacheStart = start;
        size = 0;
        long ct = sdc.count();
        for (int i=0; i<ct; i++) {
            if (start<=size&&size<start+maxSz) cache[size-start]=String(sdc.entry(i));

            size++;
        }
    }

    //open a folder
    bool goFolder(String folderName)
    {
        if (!sdc.goFolder(folderName)) return false;
        refresh();
        return true;
    }

    //count entries on folder (files and dirs)
    long count()
    {
        return size;
    }

    //get entry index by filename
    long entryIdx(String name)
    {
        idx_t sz = min(count(), (long)maxSz);
        for(int i=0; i<sz; i++)
            if(name==cache[i]) return i+cacheStart;

        long at = sdc.entryIdx(name);
        refresh(at-(maxSz>>1));
        return at;
    }

    //get folder content entry by index
    String entry(long idx)
    {
        if(0>idx || idx >= size) return "";
        if(cacheStart <= idx && idx < (cacheStart+maxSz)) return cache[idx-cacheStart];
        refresh(idx - (maxSz >> 1));
        return entry(idx);
    }
};

#define USE_BACKDOTS 1

template <typename FS>
class SDMenuT : public menuNode, public FS
{
public:
    String folderName = "/"; //set this to other folder when needed
    String selectedFolder = "/";
    String selectedFile = "";
    // using menuNode::menuNode;//do not use default constructors as we wont allocate for data
    SDMenuT(typename FS::Type &sd, constText *title, const char *at, Menu::action act = doNothing, Menu::eventMask mask = noEvent)
        : menuNode(title, 0, NULL, act, mask,wrapStyle, (systemStyles)(_menuData | _canNav)),
        FS(sd)
    {}

    void begin() { FS::goFolder(folderName); }

    //this requires latest menu version to virtualize data tables
    prompt &operator[](idx_t i) const override { return *(prompt *)this; } //this will serve both as menu and as its own prompt
    result sysHandler(SYS_FUNC_PARAMS) override
    {
        switch (event)
        {
        case enterEvent:
            if (nav.root->navFocus != nav.target)
            {                                                                                                        //on sd card entry
                nav.sel = ((SDMenuT<FS> *)(&item))->entryIdx(((SDMenuT<FS> *)(&item))->selectedFile) + USE_BACKDOTS; //restore context
                // Serial.println(nav.sel);
            }
        }
        return proceed;
    }

    void doNav(navNode &nav, navCmd cmd)
    {
        switch (cmd.cmd)
        {
        case enterCmd:
            if (nav.sel >= USE_BACKDOTS)
            {
                String selFile = SDMenuT<FS>::entry(nav.sel - USE_BACKDOTS);

                if (selFile.endsWith("/"))
                {
                    // Serial.print("\nOpen folder...");
                    //open folder (reusing the menu)
                    folderName += selFile;
                    SDMenuT<FS>::goFolder(folderName);
                    dirty = true; //redraw menu
                    nav.sel = 0;
                }
                else
                {
                    //Serial.print("\nFile selected:");
                    //select a file and return
                    selectedFile = selFile;
                    selectedFolder = folderName;
                    nav.root->node().event(enterEvent);
                    menuNode::doNav(nav, escCmd);
                }
                return;
            }
        case escCmd:
            if (folderName == "/")            //at root?
                menuNode::doNav(nav, escCmd); //then exit
            else
            { //previous folder
                idx_t at = folderName.lastIndexOf("/", folderName.length() - 2) + 1;
                String fn = folderName.substring(at, folderName.length() - 1);
                folderName.remove(folderName.lastIndexOf("/", folderName.length() - 2) + 1);
                FS::goFolder(folderName);
                dirty = true; //redraw menu
                nav.sel = FS::entryIdx(fn) + USE_BACKDOTS;
            }
            return;
        }
        menuNode::doNav(nav, cmd);
    }

    //print menu and items as this is a virtual data menu
    Used printTo(navRoot &root, bool sel, menuOut &out, idx_t idx, idx_t len, idx_t pn)
    {
        if (root.navFocus != this)
        { //show given title or filename if selected
            return selectedFile == "" ? menuNode::printTo(root, sel, out, idx, len, pn) : out.printRaw(selectedFile.c_str(), len);
        }
        else if (idx == -1)
        { //when menu open (show folder name)
            ((menuNodeShadow *)shadow)->sz = FS::count() + USE_BACKDOTS;
            idx_t at = folderName.lastIndexOf("/", folderName.length() - 2) + 1;
            String fn = folderName.substring(at, folderName.length() - 1);
            return out.printRaw(fn.c_str(), len);
            // return out.printRaw(folderName.c_str(),len);
            // return out.printRaw(SerialMenu<FS>::dir.name(),len);
        }
        //drawing options
        idx_t i = out.tops[root.level] + idx;
        if (i < USE_BACKDOTS)
            len -= out.printRaw("[..]", len);
        else
            len -= out.printRaw(FS::entry(out.tops[root.level] + idx - USE_BACKDOTS).c_str(), len);
        Serial.println("");
        Serial.println("");
        return len;
    }
};

class SerialMenu : public SDMenuT<FSO<decltype(SFL)>>
{
public:
    SerialMenu(constText *title, const char *at, Menu::action act = doNothing, Menu::eventMask mask = noEvent)
        : SDMenuT<FSO<decltype(SFL)>>(SFL, title, at, act, mask) {}
};