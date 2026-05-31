#pragma once

namespace D {

class TreeCanvas;

// Load/save appearance preferences (colors, flower radius, turtle symbols) to
// the platform-native QSettings store so they persist between sessions.
// Window geometry is handled separately by MainWindow.
void loadPreferences(TreeCanvas& canvas);
void savePreferences(const TreeCanvas& canvas);

}  // namespace D
