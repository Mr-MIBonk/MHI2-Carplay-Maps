// Project maintainer: omonob. Publication label: 2026-08-29.
package org.omonob.cpmap.ui;

import de.vw.mib.view.internal.AbstractViewHandler;
import de.vw.mib.widgets.Container;
import de.vw.mib.widgets.WidgetList;
import de.vw.mib.widgets.controller.DefaultWidgetListController;
import de.vw.mib.widgets.controller.DefaultListItemController;
import de.vw.mib.widgets.controller.DefaultPushButtonController;
import de.vw.mib.widgets.controller.DefaultPushUpController;
import de.vw.mib.widgets.controller.DefaultInactiveAreaController;
import de.vw.mib.widgets.event.OutEventListener;
import de.vw.mib.widgets.models.*;
import de.vw.mib.widgets.ui.ContainerUI;
import de.vw.mib.widgets.ui.WidgetUI;
import de.vw.mib.graphics.Color;
import java.io.File;
import java.io.FileOutputStream;
import java.lang.reflect.Array;
import java.lang.reflect.Method;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

/** MU1102 isolated native HMI experiment. No stock ASL event is repurposed. */
public final class IndependentMenuViewBridge {
    private static final Hashtable VIEWS=new Hashtable();
    // Reflective property discovery used to be repeated for every cloned
    // label/image/button on first entry. Cache immutable method pairs by class.
    private static final Hashtable COPY_PLANS=new Hashtable();
    private static final Object[] NO_ARGUMENTS=new Object[0];
    private static final boolean TRACE=new File("/tmp/cpmap-menu-trace").isFile();
    private final ProjectionSettings store;
    private final WirelessCarPlaySettings wirelessStore;
    private final org.omonob.phone.PhoneConnectionSettings phoneStore;
    private PhonePage phonePage;
    private final WidgetModel rowDonor,plainDonor;
    private final DropDownModel dropDownDonor;
    private boolean detailsReady;
    private final ListItemModel phoneEntryItem;
    private final ButtonModel phoneEntryButton;
    private final TextAreaModel phoneEntryLabel;
    private final Vector phoneEntryClones=new Vector();
    private final CardPositionSettings cardStore;
    private final ViewModel view;
    private final ContainerModel parent;
    private final WidgetModel stockList;
    private final WidgetModel stockReference;
    private final WidgetModel[] originalStockChildren;
    private final de.vw.mib.widgets.interfaces.WidgetCloneFactory originalCloneFactory;
    private final Vector entryClones=new Vector();
    private final ListItemModel entryItem;
    private final ButtonModel entryButton;
    private final TextAreaModel entryLabel;
    private TextAreaModel pageTitle;
    private Container page;
    private final int stockHeight;
    private final WidgetModel[] originalParentChildren, originalViewChildren;
    private static final int[] FIELDS={0,1,5};
    private final ButtonModel[] rows=new ButtonModel[FIELDS.length], options=new ButtonModel[8], confirmationButtons=new ButtonModel[2];
    private final TextAreaModel[] names=new TextAreaModel[FIELDS.length], values=new TextAreaModel[FIELDS.length], optionLabels=new TextAreaModel[8], confirmationLabels=new TextAreaModel[2];
    private final ListItemModel[] choiceItems=new ListItemModel[8];
    private WidgetList choiceList;
    private Container confirmation;
    private final Vector listeners=new Vector();
    private Container panel,popup;
    private DropDownModel popupBackground,confirmationBackground;
    private TextAreaModel popupTitle,notice;
    private final Hashtable copies=new Hashtable();
    private final Hashtable baseFonts=new Hashtable();
    private int nextId=1047920640, activeField=-1,pendingMap=-1;
    private long pendingGeneration;
    private String language;

    private IndependentMenuViewBridge(ViewModel target) throws Exception {
        long buildStarted=System.currentTimeMillis();
        view=target;
        store=ProjectionSettings.systemStore();
        wirelessStore=WirelessCarPlaySettings.systemStore();
        phoneStore=org.omonob.phone.PhoneConnectionSettings.systemStore();
        cardStore=CardPositionSettings.systemStore();
        store.read(); // Refuse a damaged store before touching the widget tree.
        cardStore.read();
        MenuClusterProfile.cluster();
        stockList=required(view,47234883);
        stockReference=required(view,56030257); // actual visible OEM grey bar
        originalStockChildren=((ContainerModel)stockList).getChildren();
        originalCloneFactory=((TemplateListModel)stockList).getWidgetCloneFactory();
        if(originalCloneFactory==null)throw new IllegalStateException("Native list clone factory missing");
        parent=(ContainerModel)stockList.getParent();
        stockHeight=stockList.get_height();
        originalParentChildren=parent.getChildren();
        originalViewChildren=view.getChildren();
        rowDonor=required(view,40267630);
        plainDonor=required(view,4894361);
        dropDownDonor=((DropDownButtonModel)rowDonor).get_dropDownRef();
        if(dropDownDonor==null || dropDownDonor.get_dropDownListRef()==null)throw new IllegalStateException("OEM DropDown template missing");
        // Append one real native list entry. Do not rename, hide or shrink the
        // original MirrorLink region while its page animation is being drawn.
        entryItem=(ListItemModel)cloneTree(ancestorListItem(plainDonor));
        entryButton=findButton(entryItem);entryLabel=(TextAreaModel)find(entryButton,"TextLabel");
        if(entryLabel==null)throw new IllegalStateException("Native entry label missing");
        prepareStaticItem(entryItem,entryButton);
        geometry(entryItem,30,0,1139,94);geometry(entryButton,0,0,1139,94);
        geometry(entryLabel,30,23,1024,47);resizeBackground(entryButton);
        push(entryButton,60);
        phoneEntryItem=(ListItemModel)cloneTree(ancestorListItem(plainDonor));
        phoneEntryButton=findButton(phoneEntryItem);phoneEntryLabel=(TextAreaModel)find(phoneEntryButton,"TextLabel");
        prepareStaticItem(phoneEntryItem,phoneEntryButton);
        geometry(phoneEntryItem,30,0,1139,94);geometry(phoneEntryButton,0,0,1139,94);
        geometry(phoneEntryLabel,30,23,1024,47);resizeBackground(phoneEntryButton);push(phoneEntryButton,62);
        // App-Connect entry must not synchronously clone the complete two
        // subpages and eight-item dropdown before its OEM opening animation.
        // Only two native rows are attached here. Detail widgets are created
        // once, on their own entry, on the HMI thread (never a worker thread).
        refresh();
        ((TemplateListModel)stockList).setWidgetCloneFactory(new EntryCloneFactory());
        ((ContainerModel)stockList).setChildren(append(append(originalStockChildren,entryItem),phoneEntryItem));
        log(new StringBuffer("MENU_ENTRY_BUILD_MS=").append(System.currentTimeMillis()-buildStarted).append(" lazy_detail_pages=1").toString());
    }

    private void ensureDetails() throws Exception {
        if(detailsReady)return;
        long buildStarted=System.currentTimeMillis();
        page=container("CPMAP_INDEPENDENT_SETTINGS_PAGE",0,0,view.get_width(),view.get_height());
        WidgetModel background=cloneTree(required(view,69892873));
        geometry(background,0,0,view.get_width(),view.get_height());
        InactiveAreaModel pageTouch=(InactiveAreaModel)cloneTree(((DropDownButtonModel)rowDonor).get_inactiveAreaRef());
        geometry(pageTouch,0,0,view.get_width(),view.get_height());pageTouch.set_exclusionArea(new de.vw.mib.graphics.Rectangle());
        DefaultInactiveAreaController pageTouchController=new DefaultInactiveAreaController();
        pageTouch.setController(pageTouchController);pageTouchController.setWidget(pageTouch);
        Action pageTouchAction=new Action(pageTouch,90);pageTouchController.addOutEventListener(pageTouchAction);listeners.addElement(pageTouchAction);
        ContainerModel headline=(ContainerModel)cloneTree(required(view,19482720));
        pageTitle=(TextAreaModel)find(headline,"TextLabel");
        ButtonModel pageBack=findButton(headline);push(pageBack,61);
        WidgetModel frame=cloneTree(required(view,96892073));geometry(frame,0,82,1280,16);
        panel=container("CPMAP_INDEPENDENT_ROWS",30,84,1139,549);
        WidgetModel[] rowNodes=new WidgetModel[FIELDS.length];
        for(int i=0;i<FIELDS.length;i++) {
            rows[i]=(ButtonModel)cloneTree(rowDonor);
            names[i]=(TextAreaModel)find(rows[i],"TextLabel");
            values[i]=(TextAreaModel)find(rows[i],"CurrentConnectiontype");
            if(names[i]==null || values[i]==null) throw new IllegalStateException("Exact selector labels missing");
            // Use the native selector appearance, but our own push event and
            // popup. No link to the original connection-type state machine.
            ((DropDownButtonModel)rows[i]).set_dropDownRef(null);
            ((DropDownButtonModel)rows[i]).set_inactiveAreaRef(null);
            ((DropDownButtonModel)rows[i]).set_openDropDown(false);
            geometry(rows[i],0,i*91,panel.get_width(),stockReference.get_height());
            names[i].set_x(30); names[i].set_width(510);
            values[i].set_x(600); values[i].set_width(447);
            push(rows[i],FIELDS[i]);
            rowNodes[i]=rows[i];
        }
        panel.setChildren(rowNodes);
        page.setChildren(new WidgetModel[]{background,pageTouch,frame,headline,panel});
        page.set_visible(false);
        NativePageTransition.configure(page,stockList);

        popup=container("CPMAP_NATIVE_CHOICE_POPUP",0,0,view.get_width(),view.get_height());
        InactiveAreaModel dismiss=(InactiveAreaModel)cloneTree(((DropDownButtonModel)rowDonor).get_inactiveAreaRef());
        geometry(dismiss,0,0,view.get_width(),view.get_height());
        dismiss.set_exclusionArea(new de.vw.mib.graphics.Rectangle());
        DefaultInactiveAreaController dismissController=new DefaultInactiveAreaController();
        dismiss.setController(dismissController);dismissController.setWidget(dismiss);
        Action dismissAction=new Action(dismiss,20);dismissController.addOutEventListener(dismissAction);listeners.addElement(dismissAction);
        // The source App-Connect DropDown uses the same 2789/2297/169 skin,
        // 26/23 list inset, 562px option and 91px pitch as SYSTEM_SETUP_UNITS.
        // Clone the complete native graph: never stretch a plain button as a
        // popup background, and never give each option its own grey panel.
        popupBackground=(DropDownModel)cloneTree(dropDownDonor);
        popupBackground.set_name("CPMAP_OEM_DROPDOWN");
        popupBackground.set_closeAutomatically(false);
        popupBackground.set_closeMode(PushUpModel.CLOSEMODE_IMMEDIATE);
        DefaultPushUpController popupController=new DefaultPushUpController();
        popupBackground.setController(popupController);popupController.setWidget(popupBackground);
        popupTitle=(TextAreaModel)cloneTree(required(plainDonor,101603979));
        geometry(popupTitle,620,128,580,64);
        popupTitle.set_colorForeground(Color.BLACK);
        popupTitle.set_text("");
        popupTitle.set_font(new de.vw.mib.graphics.font.FontAttribs(popupTitle.get_font().getName(),30,popupTitle.get_font().getStyle()));
        normalizeText(popupTitle,64,false);
        notice=(TextAreaModel)cloneTree(required(plainDonor,101603979));
        geometry(notice,615,480,580,82);
        notice.set_colorForeground(Color.BLACK);
        notice.set_text("");
        notice.set_font(new de.vw.mib.graphics.font.FontAttribs(notice.get_font().getName(),18,notice.get_font().getStyle()));
        normalizeText(notice,44,true);
        // Reuse the firmware's WidgetList, ListItem, UI and gesture controller.
        // A single clipped viewport replaces the former four-item pages. No
        // heading, page buttons, page number or explanatory footer is added.
        choiceList=(WidgetList)popupBackground.get_dropDownListRef();
        choiceList.set_name("CPMAP_CONTINUOUS_CHOICES");
        choiceList.setListControl(null);choiceList.setListItemTransfer(null);
        choiceList.set_cursorImageRef(null);choiceList.set_followItemRefs(null);
        choiceList.set_letterScrollingIndicatorRef(null);choiceList.set_letterScrollingTextRef(null);
        choiceList.set_listControlEnabled(false);choiceList.set_followMode(false);
        choiceList.set_dragDropEnabled(false);choiceList.set_letterScrollingEnabled(false);
        choiceList.set_listViewObserverEnabled(false);choiceList.set_dynamicItemSize(false);
        choiceList.set_cyclicScrolling(false);choiceList.set_interactive(true);
        choiceList.set_orientation(WidgetListModel.ORIENTATION_VERTICAL);
        choiceList.set_scrollTypeTouch(TemplateListModel.SCROLLTYPETOUCH_STEPLESS);
        choiceList.set_scrollTypeRotary(TemplateListModel.SCROLLTYPEROTARY_LAST_ELEMENT);
        choiceList.set_forwardPressDelay(120);
        choiceList.set_padding(new de.vw.mib.graphics.Insets(0,0,0,0));
        choiceList.set_spacingVertical(-3);choiceList.set_spacingHorizontal(0);choiceList.set_spacingPage(0);
        choiceList.set_separatorMode(WidgetListModel.SEPARATOR_MODE_ENABLED);
        geometry(choiceList,26,23,562,549);
        DefaultWidgetListController listController=new DefaultWidgetListController();
        choiceList.setController(listController);listController.setWidget(choiceList);
        WidgetModel itemDonor=findListItem(dropDownDonor.get_dropDownListRef());
        if(itemDonor==null)throw new IllegalStateException("OEM list item template missing");
        WidgetModel[] itemNodes=new WidgetModel[8];
        for(int i=0;i<8;i++) {
            choiceItems[i]=(ListItemModel)cloneTree(itemDonor);
            options[i]=findButton(choiceItems[i]);
            optionLabels[i]=(TextAreaModel)find(options[i],"RowLabel");
            if(optionLabels[i]==null) throw new IllegalStateException("Native option label missing");
            geometry(options[i],0,0,562,94);
            // Preserve the OEM 40px font and RowLabel x92/y24/w440/h47.
            // Preserve the native state image used for the current-value mark.
            push(options[i],10+i);
            choiceItems[i].set_name("CPMAP_CHOICE_ITEM");
            choiceItems[i].setListItemControl(null);choiceItems[i].set_dynamicIndex(-1);
            choiceItems[i].set_existenceFlags(null);choiceItems[i].set_subItemRefs(null);
            choiceItems[i].set_dummyContentRef(null);choiceItems[i].set_effectWidgetRefs(null);
            choiceItems[i].set_dynamicResizeWidgetRef(null);choiceItems[i].set_dynamicResizeDirection(ListItemModel.DYNAMICRESIZEDIRECTION_NONE);
            choiceItems[i].set_numberContainerRef(null);choiceItems[i].set_numberTextRef(null);
            choiceItems[i].set_selectable(true);choiceItems[i].set_draggable(false);choiceItems[i].set_lockable(false);
            choiceItems[i].set_selectedButtonRefs(new AbstractButtonModel[]{options[i]});
            // The saved value is owned by our store, not by the OEM ASL list.
            // Keep rotary focus (selected) separate from the current-value
            // checkmark (activated), including during viewport validation.
            choiceItems[i].set_activatedButtonRefs(new AbstractButtonModel[0]);
            choiceItems[i].set_activatedIndex(-1);
            geometry(choiceItems[i],0,i*91,562,94);
            DefaultListItemController itemController=new DefaultListItemController();
            choiceItems[i].setController(itemController);itemController.setWidget(choiceItems[i]);
            itemNodes[i]=choiceItems[i];
        }
        choiceList.setChildren(itemNodes);
        popupBackground.setChildren(new WidgetModel[]{choiceList});
        // Confirmation is a separate state, not a permanent row in the list.
        confirmation=container("CPMAP_MAP_PRIORITY_CONFIRM",0,0,view.get_width(),view.get_height());
        confirmationBackground=(DropDownModel)cloneTree(dropDownDonor);
        confirmationBackground.set_dropDownListRef(null);confirmationBackground.setChildren(new WidgetModel[0]);
        confirmationBackground.set_layoutAutomatically(false);confirmationBackground.set_closeAutomatically(false);
        geometry(confirmationBackground,575,101,614,426);
        WidgetModel[] confirmNodes=new WidgetModel[5];confirmNodes[0]=confirmationBackground;confirmNodes[1]=popupTitle;confirmNodes[2]=notice;
        for(int i=0;i<2;i++){
            confirmationButtons[i]=(ButtonModel)cloneTree(plainDonor);
            confirmationLabels[i]=(TextAreaModel)find(confirmationButtons[i],"TextLabel");
            geometry(confirmationButtons[i],601,196+i*80,562,76);
            geometry(confirmationLabels[i],28,0,506,76);normalizeText(confirmationLabels[i],76,false);
            confirmationLabels[i].set_font(new de.vw.mib.graphics.font.FontAttribs(confirmationLabels[i].get_font().getName(),32,confirmationLabels[i].get_font().getStyle()));
            push(confirmationButtons[i],40+i);confirmNodes[3+i]=confirmationButtons[i];
        }
        confirmation.setChildren(confirmNodes);confirmation.set_visible(false);
        popup.setChildren(new WidgetModel[]{dismiss,popupBackground,confirmation});
        popup.set_visible(false);
        phonePage=new PhonePage(rowDonor,plainDonor);
        // The generated factory knows only the firmware's original CloneData.
        // Delegate those unchanged and clone only our added static entry here.
        view.setChildren(append(append(append(originalViewChildren,page),phonePage.root),popup));
        // These roots are appended after View.init()/activate(). Initializing
        // just their WidgetList leaves PushUp's touch delegator and the native
        // button controllers uninitialized. Initialize and activate the entire
        // owned tree once; the parent View owns later deactivate/deInit calls.
        NativeDetailLifecycle.initialize(page,view);
        NativeDetailLifecycle.initialize(phonePage.root,view);
        NativeDetailLifecycle.initialize(popup,view);
        detailsReady=true;
        refresh();
        log("DETAIL_TREES_INITIALIZED roots=3 popup_touch_delegator=ready");
        log("INDEPENDENT_ENTRY_ATTACHED fields=3 stock_mirrorlink_untouched=1 wireless_golden_runtime=1 animation_selector=1 card_position_selector=1");
        log(new StringBuffer("MENU_BUILD_MS=").append(System.currentTimeMillis()-buildStarted).append(" property_plans=").append(COPY_PLANS.size()).toString());
    }

    public static void attach(AbstractViewHandler handler) {
        try {
            ViewModel view=handler.getView();
            if(view==null || !"SMI_SETUP_MAIN".equals(view.get_name()) || VIEWS.containsKey(handler)) return;
            if(new File("/tmp/cpmap-menu-baseline-only").isFile()) { log("BASELINE_ONLY"); return; }
            VIEWS.put(handler,new IndependentMenuViewBridge(view));
        } catch(Throwable error) { log(new StringBuffer("ATTACH_FAILED ").append(error).toString()); }
    }
    public static void failure(AbstractViewHandler handler,Throwable error){
        try{
            if(!"SMI_SETUP_MAIN".equals(handler.getName()))return;
            java.io.StringWriter buffer=new java.io.StringWriter();
            error.printStackTrace(new java.io.PrintWriter(buffer));
            log(new StringBuffer("NATIVE_LIFECYCLE_FAILURE ").append(buffer.toString()).toString());
        }catch(Throwable ignored){}
    }
    public static void refresh(AbstractViewHandler handler) {
        IndependentMenuViewBridge bridge=(IndependentMenuViewBridge)VIEWS.get(handler);
        if(bridge==null) return;
        try {
            // Generated activate scripts have now finished. Keep every stock
            // item and the full original viewport in the scrollable list.
            bridge.stockList.set_height(bridge.stockHeight);
            bridge.refresh();
            ((WidgetListModel)bridge.stockList).validateList();
            bridge.entryItem.repaint();
        } catch(Throwable error) { log(new StringBuffer("REFRESH_FAILED ").append(error).toString()); }
    }
    public static void detach(AbstractViewHandler handler) {
        IndependentMenuViewBridge bridge=(IndependentMenuViewBridge)VIEWS.remove(handler);
        if(bridge==null) return;
        for(int i=0;i<bridge.listeners.size();i++) {
            Action action=(Action)bridge.listeners.elementAt(i);
            action.button.getController().removeOutEventListener(action);
        }
        bridge.view.setChildren(bridge.originalViewChildren);
        bridge.parent.setChildren(bridge.originalParentChildren);
        ((ContainerModel)bridge.stockList).setChildren(bridge.originalStockChildren);
        ((TemplateListModel)bridge.stockList).setWidgetCloneFactory(bridge.originalCloneFactory);
        bridge.stockList.set_height(bridge.stockHeight);
        log("SELECTORS_DETACHED original_tree_restored=1");
    }
    private void refresh() throws Exception {
        language=UiMessages.currentLanguage();
        ProjectionSettings.State state=store.read();
        boolean rtl=UiMessages.isRightToLeft(language);
        entryLabel.set_text(MenuSectionMessages.get(language));
        phoneEntryLabel.set_text(phoneTitle());applyFont(phoneEntryLabel);phoneEntryLabel.invalidate();
        for(int n=0;n<phoneEntryClones.size();n++){TextAreaModel label=(TextAreaModel)find((WidgetModel)phoneEntryClones.elementAt(n),"TextLabel");label.set_text(phoneTitle());applyFont(label);label.invalidate();}
        for(int i=0;i<entryClones.size();i++){
            TextAreaModel label=(TextAreaModel)find((WidgetModel)entryClones.elementAt(i),"TextLabel");
            label.set_text(MenuSectionMessages.get(language));applyFont(label);label.invalidate();
        }
        applyFont(entryLabel);entryLabel.invalidate();
        if(!detailsReady)return;
        pageTitle.set_text(MenuSectionMessages.get(language));
        // Hidden phone rows are refreshed on opening; they need not perform
        // another persistent-state/font scan on every App-Connect activation.
        if(phonePage.root.is_visible())phonePage.refresh();
        applyFont(entryLabel);applyFont(pageTitle);
        entryLabel.set_alignmentHorizontal(rtl?AbstractTextModel.ALIGNMENTHORIZONTAL_RIGHT:AbstractTextModel.ALIGNMENTHORIZONTAL_LEFT);
        pageTitle.set_alignmentHorizontal(rtl?AbstractTextModel.ALIGNMENTHORIZONTAL_RIGHT:AbstractTextModel.ALIGNMENTHORIZONTAL_LEFT);
        entryLabel.invalidate();pageTitle.invalidate();
        for(int i=0;i<entryClones.size();i++){
            WidgetModel clone=(WidgetModel)entryClones.elementAt(i);
            TextAreaModel label=(TextAreaModel)find(clone,"TextLabel");
            label.set_text(MenuSectionMessages.get(language));applyFont(label);label.invalidate();
        }
        int width=stockReference.get_width(),height=stockReference.get_height();
        // The original view finalizes its anchor during activation. The panel
        // remains hidden until that phase and our final layout have completed.
        // OEM SYSTEM_SETUP_UNITS: model height94, visible height80, pitch91.
        // A 94px model height is NOT the row pitch (the skin has transparent edges).
        int x=30;
        if(x<0 || x+width>view.get_width() || height<64 || height>100) throw new IllegalStateException("OEM grey-bar bounds invalid");
        // The native WidgetList is wider than its visible grey bar. Anchor to
        // that image, not to the list or its scrolling content coordinates.
        panel.set_x(x);panel.set_y(84);panel.set_width(width);panel.set_height(549);
        popupTitle.set_width(514);notice.set_width(520);notice.set_visualLineCount(2);
        trace(new StringBuffer("OEM_ANCHOR x=").append(x).append(" width=").append(width)
            .append(" height=").append(height).append(" ref_abs_x=").append(stockReference.getAbsX()).append(" parent_abs_x=").append(parent.getAbsX()).toString());
        for(int i=0;i<FIELDS.length;i++) {
            // Reapply after native initialization/activation has performed its
            // anchor layout. Width changes must also resize the image child.
            int field=FIELDS[i];
            rows[i].set_visible(true);
            geometry(rows[i],0,i*91,width,height);
            resizeBackground(rows[i]);
            names[i].set_x(30);names[i].set_width(510);
            values[i].set_x(600);values[i].set_width(447);
            WidgetModel arrow=find(rows[i],"ArrowGlowContainer");
            if(arrow!=null)arrow.set_x(1067);
            WidgetModel separator=find(rows[i],"Separator");
            if(separator!=null)separator.set_x(569);
            names[i].set_text(field<2?LinkedMessages.get(language,field):CardMenuMessages.get(language,0));
            values[i].set_text(choice(field,field<2?state.value(field):MenuClusterProfile.cardChoice(cardStore.read().position)));
            applyFont(names[i]);applyFont(values[i]);
            names[i].set_alignmentHorizontal(rtl?AbstractTextModel.ALIGNMENTHORIZONTAL_RIGHT:AbstractTextModel.ALIGNMENTHORIZONTAL_LEFT);
            values[i].set_alignmentHorizontal(rtl?AbstractTextModel.ALIGNMENTHORIZONTAL_RIGHT:AbstractTextModel.ALIGNMENTHORIZONTAL_LEFT);
            names[i].invalidate(); values[i].invalidate(); rows[i].repaint();
            trace(new StringBuffer("ROW_GEOMETRY ").append(i).append(" button=").append(rows[i].get_x()).append(',')
                .append(rows[i].get_width()).append(" value=").append(values[i].get_x()).append(',').append(values[i].get_width())
                .append(" padding=").append(values[i].get_padding()).toString());
        }
        applyFont(popupTitle);applyFont(notice);
        int textAlignment=rtl?AbstractTextModel.ALIGNMENTHORIZONTAL_RIGHT:AbstractTextModel.ALIGNMENTHORIZONTAL_LEFT;
        popupTitle.set_alignmentHorizontal(textAlignment);
        notice.set_alignmentHorizontal(textAlignment);
        for(int option=0;option<8;option++) {
            applyFont(optionLabels[option]);
            optionLabels[option].set_alignmentHorizontal(textAlignment);
        }
        for(int option=0;option<2;option++){
            applyFont(confirmationLabels[option]);confirmationLabels[option].set_alignmentHorizontal(textAlignment);
        }
        trace(new StringBuffer("MENU_STATE locale=").append(language).append(" generation=").append(state.generation)
            .append(" map_mask=").append(state.mapMask).append(" mirror_mask=").append(state.mirrorMask)
            .append(" info=").append(state.information).append(" source=").append(state.source).toString());
    }
    private void applyFont(TextAreaModel label) {
        de.vw.mib.graphics.font.FontAttribs original=(de.vw.mib.graphics.font.FontAttribs)baseFonts.get(label);
        if(original==null){original=new de.vw.mib.graphics.font.FontAttribs(label.get_font());baseFonts.put(label,original);}
        MenuFonts.apply(label,original,language);
    }
    private String choice(int field,int value) {
        if(field>=100)return UiMessages.get(language,value==0?UiMessages.OFF:UiMessages.ON);
        if(field==5)return MenuClusterProfile.cardLabel(language,value);
        if(field==ProjectionSettings.VC_MAP)return value==1?"CarPlay Maps":LinkedMessages.get(language,LinkedMessages.OEM);
        if(field==ProjectionSettings.VC_MIRROR)return value==4?"CarPlay Mirroring":LinkedMessages.get(language,LinkedMessages.NONE);
        return "";
    }
    private void open(int field) throws Exception {
        refresh();activeField=field;pendingMap=-1;
        renderChoices();
    }
    private void renderChoices() throws Exception {
        ProjectionSettings.State state=store.read();
        int count=choiceCount(activeField),current=activeField>=100?(phoneStore.read().enabled(activeField-100)?1:0):activeField<3?state.value(activeField):activeField==3?(wirelessStore.read().enabled?1:0):MenuClusterProfile.cardChoice(cardStore.read().position);
        confirmation.set_visible(false);choiceList.set_visible(true);
        int visibleRows=Math.min(count,6);
        choiceList.set_height(visibleRows*91+3);
        popupBackground.set_height(visibleRows*91+50);
        for(int i=0;i<8;i++) {
            int selected=visibleChoice(activeField,i);
            boolean visible=i<count;
            boolean allowed=visible && (activeField!=ProjectionSettings.VC_MIRROR || ProjectionSettings.mirrorChoiceAllowed(state,selected));
            // Native disabled widgets pass their tap to the outside-dismiss
            // layer. Consume it here, render disabled, and reject in action()
            // AND in the store. A grey item must not close the popup.
            choiceItems[i].set_visible(visible);options[i].set_visible(visible);options[i].set_enabled(true);
            options[i].set_selected(false);
            optionLabels[i].set_text(visible?choice(activeField,selected):"");
            optionLabels[i].set_alpha(allowed?255:96);
            optionLabels[i].invalidate(); options[i].repaint();
        }
        // Visibility changes initialize the OEM list viewport. Reveal first,
        // then set the selected position, including the very first opening.
        popup.set_visible(true);
        choiceList.validateList();choiceList.setActivatedIndex(activeField<2?(current==(activeField==0?1:4)?0:1):current);choiceList.set_selectedIndex(activeField<2?(current==(activeField==0?1:4)?0:1):current);
        // The stock automatic anchor algorithm assumes a short ASL list. A
        // scrolling eight-item static list must stay inside the content area;
        // keep the exact OEM skin/insets/pitch, only bound its opening rect.
        popupBackground.set_layoutAutomatically(count<=6);
        if(count>6)geometry(popupBackground,575,66,614,visibleRows*91+50);
        popupBackground.setAutoLayoutArea(new de.vw.mib.graphics.Rectangle(0,84,view.get_width(),549));
        de.vw.mib.graphics.Rectangle anchor=new de.vw.mib.graphics.Rectangle();(activeField>=100?phonePage.buttons[activeField-100]:rows[activeField]).getAbsBounds(anchor);
        popupBackground.setArrowDestinationArea(anchor);
        popupBackground.set_visible(true);
        // PushUp.visibleChanged initializes the native list, resetting its
        // viewport. Restore the saved item's position after that transition.
        choiceList.setViewportY(Math.max(0,Math.min(current*91,(count-6)*91)));
        for(int i=0;i<count;i++)options[i].set_activated(visibleChoice(activeField,i)==current);
        popup.repaint();
        log(new StringBuffer("CHOICE_OPEN field=").append(activeField).append(" count=").append(count).append(" continuous_scroll=1 viewport_y=").append(choiceList.getViewport().y).toString());
    }
    private void showConfirmation(int mapChoice) throws Exception {
        ProjectionSettings.State state=store.read();pendingMap=mapChoice;pendingGeneration=state.generation;
        popupBackground.set_visible(false);confirmation.set_visible(true);confirmationBackground.set_visible(true);
        popupTitle.set_text(LinkedMessages.get(language,LinkedMessages.CONFIRM));
        geometry(notice,615,360,notice.get_width(),120);notice.set_visualLineCount(3);
        notice.set_text(new StringBuffer(LinkedMessages.get(language,LinkedMessages.ADJUST)).append('\n')
            .append(choice(ProjectionSettings.VC_MIRROR,ProjectionSettings.mirrorChoiceIndex(ProjectionSettings.mirrorAfterMapChoice(state,mapChoice)))).toString());
        for(int i=0;i<2;i++){
            confirmationLabels[i].set_text(LinkedMessages.get(language,i==0?LinkedMessages.APPLY:LinkedMessages.CANCEL));
            resizeBackground(confirmationButtons[i]);confirmationLabels[i].invalidate();confirmationButtons[i].repaint();
        }
        popupTitle.invalidate();notice.invalidate();popup.set_visible(true);popup.repaint();
        log(new StringBuffer("MAP_PRIORITY_CONFIRM map_choice=").append(mapChoice).append(" generation=").append(pendingGeneration).toString());
    }
    private void closePopup(){if(popupBackground!=null)popupBackground.set_visible(false);if(popup!=null)popup.set_visible(false);activeField=-1;pendingMap=-1;}
    private void commit(int field,int selected)throws Exception {
        if(field>=100){
            org.omonob.phone.PhoneConnectionSettings.State state=phoneStore.set(field-100,selected==1);
            org.omonob.phone.PhoneConnectionRuntime.preferencesChanged(state,field-100);
            log(new StringBuffer("OMONOB_PHONE_SETTING_SAVED field=").append(field-100).append(" generation=").append(state.generation).toString());
            closePopup();refresh();return;
        }
        if(field==5){
            CardPositionSettings.State card=cardStore.set(MenuClusterProfile.cardPosition(selected));
            log(new StringBuffer("CARD_POSITION_SAVED position=").append(card.position).append(" generation=").append(card.generation).toString());
            closePopup();refresh();return;
        }
        if(field==3){
            WirelessCarPlaySettings.State wireless=wirelessStore.set(selected==1);
            org.omonob.phone.PhoneConnectionSettings.State phone=phoneStore.set(org.omonob.phone.PhoneConnectionSettings.CARPLAY_CONNECTION,wireless.enabled);
            org.omonob.phone.PhoneConnectionRuntime.preferencesChanged(phone,org.omonob.phone.PhoneConnectionSettings.CARPLAY_CONNECTION);
            log(new StringBuffer("WIRELESS_SELECTOR_SAVED enabled=").append(wireless.enabled?1:0).append(" generation=").append(wireless.generation).append(" backend=golden_phone_runtime").toString());
            closePopup();refresh();return;
        }
        ProjectionSettings.State state=store.select(field,selected);
        log(new StringBuffer("SELECTION_SAVED field=").append(field).append(" choice=").append(selected)
            .append(" map_mask=").append(state.mapMask).append(" mirror_mask=").append(state.mirrorMask)
            .append(" info=").append(state.information).append(" generation=").append(state.generation).toString());
        closePopup();refresh();
    }
    private void action(int action) {
        try {
            if(action==62){ensureDetails();closePopup();phonePage.open();return;}
            if(action==63){closePopup();NativePageTransition.hide(phonePage.root);return;}
            if(action==200){open(100);return;}
            if(action==3)return;
            if(action==60){ensureDetails();refresh();NativePageTransition.show(page);log("INDEPENDENT_PAGE_OPEN");return;}
            if(action==61){closePopup();NativePageTransition.hide(page);log("INDEPENDENT_PAGE_CLOSE");return;}
            if(action==90)return;
            if(action==0||action==1||action==5){open(action);return;}
            if(action==20){closePopup();return;}
            if(activeField<0)return;
            if(pendingMap>=0){
                if(action==41){closePopup();return;}
                if(action!=40)return;
                if(store.read().generation!=pendingGeneration){showConfirmation(pendingMap);return;}
                commit(ProjectionSettings.VC_MAP,pendingMap);return;
            }
            if(action<10 || action>17)return;
            int selected=action-10;
            if(selected>=choiceCount(activeField))return;
            selected=visibleChoice(activeField,selected);
            if(activeField>=100){commit(activeField,selected);return;}
            ProjectionSettings.State old=store.read();
            if(activeField==ProjectionSettings.VC_MIRROR && !ProjectionSettings.mirrorChoiceAllowed(old,selected)){
                log("CONFLICTING_MIRROR_IGNORED");return;
            }
            if(activeField==ProjectionSettings.VC_MAP && ProjectionSettings.mirrorAfterMapChoice(old,selected)!=old.mirrorMask){showConfirmation(selected);return;}
            commit(activeField,selected);
        } catch(Throwable error) {
            notice.set_text(UiMessages.get(language,UiMessages.SAVE_FAILED)); notice.invalidate();
            log(new StringBuffer("SELECTION_FAILED ").append(error).toString());
        }
    }
    private static int visibleChoice(int field,int index){
        if(field==ProjectionSettings.VC_MAP)return index==0?1:3;
        if(field==ProjectionSettings.VC_MIRROR)return index==0?4:7;
        return index;
    }
    private static int choiceCount(int field){return field>=100||field<2?2:field==5?MenuClusterProfile.cardCount():field==3?2:ProjectionSettings.choiceCount(field);}
    private final class EntryCloneFactory implements de.vw.mib.widgets.interfaces.WidgetCloneFactory{
        public de.vw.mib.widgets.interfaces.WidgetCloneable createWidgetCloneable(de.vw.mib.widgets.interfaces.WidgetCloneable source){
            if(source!=entryItem&&source!=phoneEntryItem)return originalCloneFactory.createWidgetCloneable(source);
            boolean phone=source==phoneEntryItem;
            try{
                ListItemModel item=(ListItemModel)cloneTree(phone?phoneEntryItem:entryItem);
                ButtonModel button=findButton(item);prepareStaticItem(item,button);push(button,phone?62:60);
                if(phone)phoneEntryClones.addElement(item);else entryClones.addElement(item);
                log("INDEPENDENT_ENTRY_NATIVE_CLONE_CREATED");
                return item;
            }catch(Exception error){throw new IllegalStateException(error.toString());}
        }
        public void destroyWidgetCloneable(de.vw.mib.widgets.interfaces.WidgetCloneable clone){
            if(phoneEntryClones.removeElement(clone))return;
            if(entryClones.removeElement(clone)){log("INDEPENDENT_ENTRY_NATIVE_CLONE_DESTROYED");return;}
            originalCloneFactory.destroyWidgetCloneable(clone);
        }
    }
    private static ListItemModel ancestorListItem(WidgetModel widget){
        for(int i=0;i<12 && widget!=null;i++,widget=widget.getParent())if(widget instanceof ListItemModel)return (ListItemModel)widget;
        throw new IllegalStateException("Native list-item ancestor missing");
    }
    private static void prepareStaticItem(ListItemModel item,ButtonModel button){
        item.setListItemControl(null);item.set_dynamicIndex(-1);item.set_existenceFlags(null);
        item.set_subItemRefs(null);item.set_dummyContentRef(null);item.set_effectWidgetRefs(null);
        item.set_dynamicResizeWidgetRef(null);item.set_dynamicResizeDirection(ListItemModel.DYNAMICRESIZEDIRECTION_NONE);
        item.set_numberContainerRef(null);item.set_numberTextRef(null);item.set_activatedIndex(-1);
        item.set_selectable(true);item.set_draggable(false);item.set_lockable(false);
        item.set_selectedButtonRefs(new AbstractButtonModel[]{button});item.set_activatedButtonRefs(new AbstractButtonModel[0]);
        DefaultListItemController controller=new DefaultListItemController();item.setController(controller);controller.setWidget(item);
    }
    private void push(ButtonModel button,int code) {
        DefaultPushButtonController controller=new DefaultPushButtonController();
        button.setController(controller); controller.setWidget(button);
        button.set_predictiveActivation(false);
        button.set_buttonMode(ButtonModel.BUTTONMODE_DEFAULT);
        Action listener=new Action(button,code);
        controller.addOutEventListener(listener); listeners.addElement(listener);
    }
    private final class Action implements OutEventListener {
        final WidgetModel button; final int code;
        Action(WidgetModel b,int c) { button=b;code=c; }
        public void outHMIEventFired(String event) { if("released".equals(event)) action(code); }
        public void outIntegerEventFired(String e,int v) { }
        public void outBooleanEventFired(String e,boolean v) { }
        public void outStringEventFired(String e,String v) { }
        public void outPointEventFired(String e,int a,int b,int c,int d) { }
        public void outListItemEventFired(String e,int a,int b,int c,int d,String s,long n) { }
        public void outSpellerEventFired(String e,String s,int a,int b,int c) { }
    }
    private Container container(String name,int x,int y,int w,int h) {
        Container result=new Container(); ContainerUI ui=new ContainerUI();
        result.setUI(ui); ui.setWidget(result); result.set_name(name); result.set_targetId(nextId++);
        geometry(result,x,y,w,h); return result;
    }
    private static void geometry(WidgetModel widget,int x,int y,int w,int h) {
        widget.set_enabled(true); widget.set_visible(true); widget.set_availableInVariant(true);
        widget.set_layoutable(false); widget.set_alpha(255); widget.set_x(x); widget.set_y(y);
        widget.set_width(w); widget.set_height(h);
    }
    private static void normalizeText(TextAreaModel text,int height,boolean multiline){
        text.set_height(height);text.set_baselineOffset(0);text.set_lineSpacing(0);
        text.set_padding(new de.vw.mib.graphics.Insets(0,0,0,0));
        text.set_alignmentVertical(multiline?TextAreaModel.ALIGNMENTVERTICAL_TOP:TextAreaModel.ALIGNMENTVERTICAL_CENTER);
        text.set_lineBreakMode(multiline?TextAreaModel.LINEBREAKMODE_MANUAL:TextAreaModel.LINEBREAKMODE_NONE);
        text.set_visualLineCount(multiline?2:1);
    }
    private static void hideChildren(WidgetModel root) {
        if(root instanceof ContainerModel) {
            WidgetModel[] children=((ContainerModel)root).getChildren();
            for(int i=0;children!=null && i<children.length;i++) children[i].set_visible(false);
        }
    }
    private static void resizeBackground(ButtonModel button) {
        WidgetModel image=find(button,"ScaleImage");
        if(image!=null) geometry(image,0,0,button.get_width(),button.get_height());
    }
    private static WidgetModel[] append(WidgetModel[] original,WidgetModel added) {
        WidgetModel[] result=new WidgetModel[original.length+1];
        System.arraycopy(original,0,result,0,original.length); result[original.length]=added; return result;
    }
    private static WidgetModel required(WidgetModel root,int id) {
        WidgetModel result=findId(root,id);
        if(result==null) throw new IllegalStateException(new StringBuffer("Missing stock template ").append(id).toString());
        return result;
    }
    private static int relativeX(WidgetModel model,WidgetModel ancestor) {
        int x=0;
        for(int depth=0;depth<16;depth++) {
            if(model==ancestor)return x;
            if(model==null)break;
            x+=model.get_x();model=model.getParent();
        }
        throw new IllegalStateException("OEM reference is outside native content container");
    }
    private static WidgetModel findId(WidgetModel root,int id) {
        if(root.get_targetId()==id) return root;
        if(root instanceof ContainerModel) {
            WidgetModel[] children=((ContainerModel)root).getChildren();
            for(int i=0;children!=null && i<children.length;i++) {
                WidgetModel result=findId(children[i],id); if(result!=null) return result;
            }
        }
        return null;
    }
    private static WidgetModel findListItem(WidgetModel root){
        if(root instanceof ListItemModel)return root;
        if(root instanceof ContainerModel){
            WidgetModel[] children=((ContainerModel)root).getChildren();
            for(int i=0;children!=null && i<children.length;i++){
                WidgetModel result=findListItem(children[i]);if(result!=null)return result;
            }
        }
        return null;
    }
    private static ButtonModel findButton(WidgetModel root){
        if(root instanceof ButtonModel)return (ButtonModel)root;
        if(root instanceof ContainerModel){
            WidgetModel[] children=((ContainerModel)root).getChildren();
            for(int i=0;children!=null && i<children.length;i++){
                ButtonModel result=findButton(children[i]);if(result!=null)return result;
            }
        }
        return null;
    }
    private static WidgetModel find(WidgetModel root,String name) {
        if(name.equals(root.get_name())) return root;
        if(root instanceof ContainerModel) {
            WidgetModel[] children=((ContainerModel)root).getChildren();
            for(int i=0;children!=null && i<children.length;i++) {
                WidgetModel result=find(children[i],name); if(result!=null) return result;
            }
        }
        return null;
    }
    private WidgetModel cloneTree(WidgetModel source) throws Exception {
        copies.clear(); collect(source);
        Enumeration keys=copies.keys();
        while(keys.hasMoreElements()) {
            WidgetModel from=(WidgetModel)keys.nextElement(), to=(WidgetModel)copies.get(from);
            Method[][] plan=copyPlan(from.getClass());
            for(int i=0;i<plan.length;i++)
                plan[i][1].invoke(to,new Object[]{remap(plan[i][0].invoke(from,NO_ARGUMENTS))});
            to.set_name(from.get_name()); to.set_targetId(nextId++);
            if(from.getUI()!=null) { WidgetUI ui=(WidgetUI)from.getUI().getClass().newInstance(); to.setUI(ui);ui.setWidget(to); }
            if(from instanceof ContainerModel) ((ContainerModel)to).setChildren((WidgetModel[])remap(((ContainerModel)from).getChildren()));
            to.enablePropertyChanged();
        }
        return (WidgetModel)copies.get(source);
    }
    private static synchronized Method[][] copyPlan(Class type) {
        Method[][] cached=(Method[][])COPY_PLANS.get(type);
        if(cached!=null)return cached;
        Method[] methods=type.getMethods();Vector pairs=new Vector();
        for(int i=0;i<methods.length;i++) {
            Method getter=methods[i];String name=getter.getName();
            int prefix=name.startsWith("get_")?4:name.startsWith("is_")?3:0;
            if(prefix==0 || getter.getParameterTypes().length!=0 || name.equals("is_activated"))continue;
            try {
                Method setter=type.getMethod(new StringBuffer("set_").append(name.substring(prefix)).toString(),new Class[]{getter.getReturnType()});
                pairs.addElement(new Method[]{getter,setter});
            }catch(NoSuchMethodException ignored) { }
        }
        cached=new Method[pairs.size()][];pairs.copyInto(cached);COPY_PLANS.put(type,cached);return cached;
    }
    private void collect(WidgetModel from) throws Exception {
        WidgetModel to=(WidgetModel)from.getClass().newInstance();to.disablePropertyChanged();copies.put(from,to);
        if(from instanceof ContainerModel) {
            WidgetModel[] children=((ContainerModel)from).getChildren();
            for(int i=0;children!=null && i<children.length;i++) collect(children[i]);
        }
    }
    private Object remap(Object value) {
        if(value instanceof WidgetModel) return copies.get(value);
        if(value instanceof WidgetModel[]) {
            WidgetModel[] old=(WidgetModel[])value;
            Object array=Array.newInstance(value.getClass().getComponentType(),old.length);
            for(int i=0;i<old.length;i++) Array.set(array,i,copies.get(old[i]));return array;
        }
        return value;
    }
    private static void log(String line) {
        try {
            if(new File("/tmp/cpmap-menu-offline.log").length()>131072) return;
            FileOutputStream out=new FileOutputStream("/tmp/cpmap-menu-offline.log",true);
            try { out.write(new StringBuffer(line).append('\n').toString().getBytes("UTF-8")); } finally { out.close(); }
        } catch(Throwable ignored) { }
    }
    private static void trace(String line){if(TRACE)log(line);}

    // Project maintainer: omonob. Publication label: 2026-08-29.
    private final class PhonePage {
        final Container root;
        final WidgetList list;
        final TextAreaModel title;
        final ListItemModel[] items=new ListItemModel[1];
        final ButtonModel[] buttons=new ButtonModel[1];
        final TextAreaModel[] labels=new TextAreaModel[1],current=new TextAreaModel[1];
        final Vector clones=new Vector();
        PhonePage(WidgetModel rowDonor,WidgetModel plainDonor)throws Exception {
            root=container("OMONOB_PHONE_SETTINGS",0,0,view.get_width(),view.get_height());
            WidgetModel background=cloneTree(required(view,69892873));
            geometry(background,0,0,view.get_width(),view.get_height());
            ContainerModel headline=(ContainerModel)cloneTree(required(view,19482720));
            title=(TextAreaModel)find(headline,"TextLabel");push(findButton(headline),63);
            WidgetModel frame=cloneTree(required(view,96892073));geometry(frame,0,82,1280,16);
            list=(WidgetList)cloneTree(stockList);list.set_name("OMONOB_PHONE_SETTINGS_LIST");
            list.setListControl(null);list.setListItemTransfer(null);list.set_listControlEnabled(false);
            list.set_followMode(false);list.set_dynamicItemSize(false);list.set_cyclicScrolling(false);
            list.set_dragDropEnabled(false);list.set_letterScrollingEnabled(false);list.set_listViewObserverEnabled(false);
            list.set_interactive(true);list.set_orientation(WidgetListModel.ORIENTATION_VERTICAL);
            list.set_scrollTypeTouch(TemplateListModel.SCROLLTYPETOUCH_STEPLESS);
            list.set_scrollTypeRotary(TemplateListModel.SCROLLTYPEROTARY_LAST_ELEMENT);
            list.set_padding(new de.vw.mib.graphics.Insets(0,0,0,0));list.set_spacingVertical(-3);
            list.set_spacingHorizontal(0);list.set_spacingPage(0);geometry(list,30,84,1139,549);
            WidgetModel[] children=new WidgetModel[1];
            for(int i=0;i<1;i++) {
                items[i]=(ListItemModel)cloneTree(ancestorListItem(rowDonor));
                buttons[i]=findButton(items[i]);prepareStaticItem(items[i],buttons[i]);
                if(buttons[i] instanceof DropDownButtonModel){
                    ((DropDownButtonModel)buttons[i]).set_dropDownRef(null);
                    ((DropDownButtonModel)buttons[i]).set_inactiveAreaRef(null);
                    ((DropDownButtonModel)buttons[i]).set_openDropDown(false);
                }
                labels[i]=(TextAreaModel)find(buttons[i],"TextLabel");
                current[i]=(TextAreaModel)find(buttons[i],"CurrentConnectiontype");
                if(labels[i]==null||current[i]==null)throw new IllegalStateException("phone_selector_labels_missing");
                geometry(items[i],0,i*91,1139,94);geometry(buttons[i],0,0,1139,94);
                geometry(labels[i],30,23,720,47);geometry(current[i],800,23,280,47);
                resizeBackground(buttons[i]);push(buttons[i],200+i);children[i]=items[i];
            }
            list.setChildren(children);
            list.setWidgetCloneFactory(new de.vw.mib.widgets.interfaces.WidgetCloneFactory(){
                public de.vw.mib.widgets.interfaces.WidgetCloneable createWidgetCloneable(de.vw.mib.widgets.interfaces.WidgetCloneable source){
                    for(int i=0;i<1;i++)if(source==items[i])try{
                        ListItemModel item=(ListItemModel)cloneTree(items[i]);ButtonModel button=findButton(item);
                        prepareStaticItem(item,button);push(button,200+i);clones.addElement(new Object[]{item,new Integer(i)});return item;
                    }catch(Exception e){throw new IllegalStateException(e.toString());}
                    throw new IllegalArgumentException("unknown_phone_row_clone");
                }
                public void destroyWidgetCloneable(de.vw.mib.widgets.interfaces.WidgetCloneable value){
                    for(int i=0;i<clones.size();i++)if(((Object[])clones.elementAt(i))[0]==value){clones.removeElementAt(i);return;}
                }
            });
            PhonePageViewport.resetToTop(list);
            root.setChildren(new WidgetModel[]{background,list,headline,frame});root.set_visible(false);
            NativePageTransition.configure(root,stockList);
        }
        void refresh()throws Exception{
            org.omonob.phone.PhoneConnectionSettings.State state=phoneStore.read();
            title.set_text(phoneTitle());applyFont(title);title.invalidate();
            for(int i=0;i<1;i++){update(items[i],i,state);buttons[i].repaint();}
            for(int i=0;i<clones.size();i++){
                Object[] pair=(Object[])clones.elementAt(i);update((WidgetModel)pair[0],((Integer)pair[1]).intValue(),state);
            }
        }
        void update(WidgetModel item,int index,org.omonob.phone.PhoneConnectionSettings.State state){
            TextAreaModel name=(TextAreaModel)find(item,"TextLabel"),value=(TextAreaModel)find(item,"CurrentConnectiontype");
            name.set_text(phoneLabel(index));value.set_text(UiMessages.get(language,state.enabled(index)?UiMessages.ON:UiMessages.OFF));
            applyFont(name);applyFont(value);name.invalidate();value.invalidate();item.repaint();
        }
        void open()throws Exception{refresh();PhonePageViewport.resetToTop(list);list.validateList();NativePageTransition.show(root);log("OMONOB_PHONE_PAGE_OPEN fields=1 viewport_reset=top initialized=whole_tree");}
    }
    private String phoneTitle(){return PhoneMenuMessages.title(language);}
    private String phoneLabel(int field){
        return PhoneMenuMessages.label(language,field);
    }

}
