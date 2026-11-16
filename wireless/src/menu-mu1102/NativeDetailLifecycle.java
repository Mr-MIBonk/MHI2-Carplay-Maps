package org.omonob.cpmap.ui;

import de.vw.mib.widgets.models.WidgetModel;
import de.vw.mib.widgets.models.ViewModel;

/** Lifecycle for newly appended native trees, called once by ensureDetails.
 * Container.init/activate recursively initializes children and their native
 * controllers. Do not reinitialize a list on every popup opening. Subsequent
 * deactivation/deinitialization belongs to the parent View's normal lifecycle.
 */
public final class NativeDetailLifecycle {
    private NativeDetailLifecycle() { }
    public static void initialize(WidgetModel root,ViewModel view) {
        if(root==null || view==null)throw new IllegalArgumentException("owned_tree_and_view_required");
        root.init(view);
        root.activate();
    }
}
