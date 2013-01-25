/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef _KMESSAGEBOX_H_
#define _KMESSAGEBOX_H_

#include <tqstring.h>
#include <tqmessagebox.h>

#include <kguiitem.h>
#include <kstdguiitem.h>

class TQWidget;
class TQStringList;
class TDEConfig;
class KDialogBase;

 /**
  * Easy message dialog box.
  *
  * Provides convenience functions for some i18n'ed standard dialogs.
  *
  * The text in message boxes is wrapped automatically. The text may either
  * be plaintext or richtext. If the text is plaintext, a newline-character
  * may be used to indicate the end of a paragraph.
  *
  * @author Waldo Bastian (bastian@kde.org)
  */
class TDEUI_EXPORT KMessageBox
{
public:
  /**
   * Button types.
   **/
 enum ButtonCode
 {
   Ok = 1,
   Cancel = 2,
   Yes = 3,
   No = 4,
   Continue = 5
 };

 enum DialogType
 {
   QuestionYesNo = 1,
   WarningYesNo = 2,
   WarningContinueCancel = 3,
   WarningYesNoCancel = 4,
   Information = 5,
   // Reserved for: SSLMessageBox = 6
   Sorry = 7,
   Error = 8,
   QuestionYesNoCancel = 9
 };

 enum OptionsType
 {
   Notify = 1,        ///< Emit a KNotifyClient event
   AllowLink = 2,     ///< The message may contain links.
   Dangerous = 4,     ///< The action to be confirmed by the dialog is a potentially destructive one
   PlainCaption = 8,  /**< Do not use TDEApplication::makeStdCaption()
   		           @since 3.4
		       */
   NoExec = 16        /**< Do not call exec() in createKMessageBox()
			   @since 3.4
		       */
 };

 /**
  * Display a simple "question" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  *
  * To be used for questions like "Do you have a printer?"
  *
  * The default button is "Yes". Pressing "Esc" selects "No".
  */

 static int questionYesNo(TQWidget *parent,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo =  KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);
 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int questionYesNoWId(WId parent_id,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo =  KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);
 /**
  * Display a simple "question" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  *
  * To be used for questions like "Do you want to discard the message or save it for later?",
  *
  * The default button is "Yes". Pressing "Esc" selects "Cancel".
  *
  * NOTE: The cancel button will always have the i18n'ed text '&Cancel'.
  */

  static int questionYesNoCancel(TQWidget *parent,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo  = KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static int questionYesNoCancelWId(WId parent_id,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo  = KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);

 /**
  * Display a "question" dialog with a listbox to show information to the user
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the list is
  *                empty, it doesn't show any listbox, working as questionYesNo.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  *
  * To be used for questions like "Do you really want to delete these files?"
  * And show the user exactly which files are going to be deleted in case
  * he presses "Yes"
  *
  * The default button is "Yes". Pressing "Esc" selects "No".
  */

 static int questionYesNoList(TQWidget *parent,
                          const TQString &text,
                          const TQStringList &strlist,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo = KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int questionYesNoListWId(WId parent_id,
                          const TQString &text,
                          const TQStringList &strlist,
                          const TQString &caption = TQString::null,
                          const KGuiItem &buttonYes = KStdGuiItem::yes(),
                          const KGuiItem &buttonNo = KStdGuiItem::no(),
                          const TQString &dontAskAgainName = TQString::null,
                          int options = Notify);

 /**
  * Display a "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  @p Yes is returned if the Yes-button is pressed. @p No is returned
  *          if the No-button is pressed.
  *
  * To be used for questions "Shall I update your configuration?"
  * The text should explain the implication of both options.
  *
  * The default button is "No". Pressing "Esc" selects "No".
  */
 static int warningYesNo(TQWidget *parent,
                         const TQString &text,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonYes = KStdGuiItem::yes(),
                         const KGuiItem &buttonNo = KStdGuiItem::no(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify | Dangerous);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int warningYesNoWId(WId parent_id,
                         const TQString &text,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonYes = KStdGuiItem::yes(),
                         const KGuiItem &buttonNo = KStdGuiItem::no(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify | Dangerous);

 /**
  * Display a "warning" dialog with a listbox to show information to the user
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the list is
  *                empty, it doesn't show any listbox, working as questionYesNo.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Question").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  'Yes' is returned if the Yes-button is pressed. 'No' is returned
  *          if the No-button is pressed.
  *
  * To be used for questions like "Do you really want to delete these files?"
  * And show the user exactly which files are going to be deleted in case
  * he presses "Yes"
  *
  * The default button is "No". Pressing "Esc" selects "No".
  */

 static int warningYesNoList(TQWidget *parent,
                            const TQString &text,
                            const TQStringList &strlist,
                            const TQString &caption = TQString::null,
                            const KGuiItem &buttonYes = KStdGuiItem::yes(),
                            const KGuiItem &buttonNo = KStdGuiItem::no(),
                            const TQString &dontAskAgainName = TQString::null,
                            int options = Notify | Dangerous);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int warningYesNoListWId(WId parent_id,
                            const TQString &text,
                            const TQStringList &strlist,
                            const TQString &caption = TQString::null,
                            const KGuiItem &buttonYes = KStdGuiItem::yes(),
                            const KGuiItem &buttonNo = KStdGuiItem::no(),
                            const TQString &dontAskAgainName = TQString::null,
                            int options = Notify | Dangerous);

 /**
  * Display a "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonContinue The text for the first button.
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * The second button always has the text "Cancel".
  *
  * @return  @p Continue is returned if the Continue-button is pressed.
  *          @p Cancel is returned if the Cancel-button is pressed.
  *
  * To be used for questions like "You are about to Print. Are you sure?"
  * the continueButton should then be labeled "Print".
  *
  * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
  */
 static int warningContinueCancel(TQWidget *parent,
                         const TQString &text,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int warningContinueCancelWId(WId parent_id,
                         const TQString &text,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify);

 /**
  * Display a "warning" dialog with a listbox to show information to the user.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the
  *                list is empty, it doesn't show any listbox, working
  *                as warningContinueCancel.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonContinue The text for the first button.
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further confirmation can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  *
  * @param options  see OptionsType
  *
  * The second button always has the text "Cancel".
  *
  * @return  @p Continue is returned if the Continue-button is pressed.
  *          @p Cancel is returned if the Cancel-button is pressed.
  *
  * To be used for questions like "You are about to Print. Are you sure?"
  * the continueButton should then be labeled "Print".
  *
  * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
  */
 static int warningContinueCancelList(TQWidget *parent,
                         const TQString &text,
                         const TQStringList &strlist,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
 static int warningContinueCancelListWId(WId parent_id,
                         const TQString &text,
                         const TQStringList &strlist,
                         const TQString &caption = TQString::null,
                         const KGuiItem &buttonContinue = KStdGuiItem::cont(),
                         const TQString &dontAskAgainName = TQString::null,
                         int options = Notify);

 /**
  * Display a Yes/No/Cancel "warning" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further questions can be turned off. If turned off
  *                all questions will be automatically answered with the
  *                last answer (either Yes or No).
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  @p Yes is returned if the Yes-button is pressed. @p No is returned
  *          if the No-button is pressed. @p Cancel is retunred if the Cancel-
  *          button is pressed.
  *
  * To be used for questions "Do you want to save your changes?"
  * The text should explain the implication of choosing 'No'.
  *
  * The default button is "Yes". Pressing "Esc" selects "Cancel"
  *
  * NOTE: The cancel button will always have the i18n'ed text '&Cancel'.
  */

  static int warningYesNoCancel(TQWidget *parent,
                                const TQString &text,
                                const TQString &caption = TQString::null,
                                const KGuiItem &buttonYes = KStdGuiItem::yes(),
                                const KGuiItem &buttonNo = KStdGuiItem::no(),
                                const TQString &dontAskAgainName = TQString::null,
                                int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static int warningYesNoCancelWId(WId parent_id,
                                const TQString &text,
                                const TQString &caption = TQString::null,
                                const KGuiItem &buttonYes = KStdGuiItem::yes(),
                                const KGuiItem &buttonNo = KStdGuiItem::no(),
                                const TQString &dontAskAgainName = TQString::null,
                                int options = Notify);

 /**
  * Display a Yes/No/Cancel "warning" dialog with a listbox to show information
  * to the user.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the
  *                list is empty, it doesn't show any listbox, working
  *                as warningYesNoCancel.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Warning").
  * @param buttonYes The text for the first button.
  *                  The default is i18n("&Yes").
  * @param buttonNo  The text for the second button.
  *                  The default is i18n("&No").
  * @param dontAskAgainName If provided, a checkbox is added with which
  *                further questions can be turned off. If turned off
  *                all questions will be automatically answered with the
  *                last answer (either Yes or No).
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  *                If @p dontAskAgainName starts with a ':' then the setting
  *                is stored in the global config file.
  * @param options  see OptionsType
  *
  * @return  @p Yes is returned if the Yes-button is pressed. @p No is returned
  *          if the No-button is pressed. @p Cancel is retunred if the Cancel-
  *          button is pressed.
  *
  * To be used for questions "Do you want to save your changes?"
  * The text should explain the implication of choosing 'No'.
  *
  * The default button is "Yes". Pressing "Esc" selects "Cancel"
  *
  * NOTE: The cancel button will always have the i18n'ed text '&Cancel'.
  *
  * @since 3.2
  */
  static int warningYesNoCancelList(TQWidget *parent,
                                const TQString &text,
                                const TQStringList &strlist,
                                const TQString &caption = TQString::null,
                                const KGuiItem &buttonYes = KStdGuiItem::yes(),
                                const KGuiItem &buttonNo = KStdGuiItem::no(),
                                const TQString &dontAskAgainName = TQString::null,
                                int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static int warningYesNoCancelListWId(WId parent_id,
                                const TQString &text,
                                const TQStringList &strlist,
                                const TQString &caption = TQString::null,
                                const KGuiItem &buttonYes = KStdGuiItem::yes(),
                                const KGuiItem &buttonNo = KStdGuiItem::no(),
                                const TQString &dontAskAgainName = TQString::null,
                                int options = Notify);

 /**
  * Display an "Error" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  * @param options  see OptionsType
  *
  * Your program messed up and now it's time to inform the user.
  * To be used for important things like "Sorry, I deleted your hard disk."
  *
  * If your program detects the action specified by the user is somehow
  * not allowed, this should never be reported with error(). Use sorry()
  * instead to explain to the user that this action is not allowed.
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The OK button will always have the i18n'ed text '&OK'.
  */

  static void error(TQWidget *parent,
                    const TQString &text,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  *
  * @since 3.2
  */
  static void errorWId(WId parent_id,
                    const TQString &text,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * Display an "Error" dialog with a listbox.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the
  *                list is empty, it doesn't show any listbox, working
  *                as error().
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  * @param options  see OptionsType
  *
  * Your program messed up and now it's time to inform the user.
  * To be used for important things like "Sorry, I deleted your hard disk."
  *
  * If your program detects the action specified by the user is somehow
  * not allowed, this should never be reported with error(). Use sorry()
  * instead to explain to the user that this action is not allowed.
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The OK button will always have the i18n'ed text '&OK'.
  *
  * @since 3.4
  */

  static void errorList(TQWidget *parent,
                    const TQString &text,
                    const TQStringList &strlist,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.4
  */

  static void errorListWId(WId parent_id,
                    const TQString &text,
                    const TQStringList &strlist,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * Displays an "Error" dialog with a "Details >>" button.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param details Detailed message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Error").
  * @param options  see OptionsType
  *
  * Your program messed up and now it's time to inform the user.
  * To be used for important things like "Sorry, I deleted your hard disk."
  *
  * The @p details message can conatin additional information about
  * the problem and can be shown on request to advanced/interested users.
  *
  * If your program detects the action specified by the user is somehow
  * not allowed, this should never be reported with error(). Use sorry()
  * instead to explain to the user that this action is not allowed.
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The OK button will always have the i18n'ed text '&OK'.
  */
  static void detailedError(TQWidget *parent,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void detailedErrorWId(WId parent_id,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null,
                    int options = Notify);

  /**
   * Like detailedError
   *
   * This function will return immediately, the messagebox will be shown
   * once the application enters an event loop and no other messagebox
   * is being shown.
   *
   * Note that if the parent gets deleted, the messagebox will not be
   * shown.
   */
  static void queuedDetailedError( TQWidget *parent,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void queuedDetailedErrorWId( WId parent_id,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null);

 /**
  * Display an "Sorry" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Sorry").
  * @param options  see OptionsType
  *
  * Either your program messed up and asks for understanding
  * or your user did something stupid.
  *
  * To be used for small problems like
  * "Sorry, I can't find the file you specified."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The ok button will always have the i18n'ed text '&OK'.
  */

  static void sorry(TQWidget *parent,
                    const TQString &text,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void sorryWId(WId parent_id,
                    const TQString &text,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * Displays a "Sorry" dialog with a "Details >>" button.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param details Detailed message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Sorry").
  * @param options  see OptionsType
  *
  * Either your program messed up and asks for understanding
  * or your user did something stupid.
  *
  * To be used for small problems like
  * "Sorry, I can't find the file you specified."
  *
  * And then @p details can contain something like
  * "foobar.txt was not found in any of
  *  the following directories:
  *  /usr/bin,/usr/local/bin,/usr/sbin"
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  * NOTE: The ok button will always have the i18n'ed text '&OK'.
  */

  static void detailedSorry(TQWidget *parent,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void detailedSorryWId(WId parent_id,
                    const TQString &text,
                    const TQString &details,
                    const TQString &caption = TQString::null,
                    int options = Notify);

 /**
  * Display an "Information" dialog.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  * @param dontShowAgainName If provided, a checkbox is added with which
  *                further notifications can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  * @param options  see OptionsType
  *
  *
  * Your program wants to tell the user something.
  * To be used for things like:
  * "Your bookmarks have been rearranged."
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  *  NOTE: The OK button will always have the i18n'ed text '&OK'.
  */

  static void information(TQWidget *parent,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const TQString &dontShowAgainName = TQString::null,
                          int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void informationWId(WId parent_id,
                          const TQString &text,
                          const TQString &caption = TQString::null,
                          const TQString &dontShowAgainName = TQString::null,
                          int options = Notify);

 /**
  * Display an "Information" dialog with a listbox.
  *
  * @param parent  If @p parent is 0, then the message box becomes an
  *                application-global modal dialog box. If @p parent is a
  *                widget, the message box becomes modal relative to parent.
  * @param text    Message string.
  * @param strlist List of strings to be written in the listbox. If the
  *                list is empty, it doesn't show any listbox, working
  *                as information.
  * @param caption Message box title. The application name is added to
  *                the title. The default title is i18n("Information").
  * @param dontShowAgainName If provided, a checkbox is added with which
  *                further notifications can be turned off.
  *                The string is used to lookup and store the setting
  *                in the applications config file.
  *                The setting is stored in the "Notification Messages" group.
  * @param options  see OptionsType
  *
  *
  * Your program wants to tell the user something.
  * To be used for things like:
  * "The following bookmarks have been rearranged:"
  *
  * The default button is "&OK". Pressing "Esc" selects the OK-button.
  *
  *  NOTE: The OK button will always have the i18n'ed text '&OK'.
  * @since 3.1
  */

  static void informationList(TQWidget *parent,
			      const TQString &text,
			      const TQStringList & strlist,
			      const TQString &caption = TQString::null,
			      const TQString &dontShowAgainName = TQString::null,
			      int options = Notify);

 /**
  * This function accepts the window id of the parent window, instead
  * of TQWidget*. It should be used only when necessary.
  * @since 3.2
  */
  static void informationListWId(WId parent_id,
			      const TQString &text,
			      const TQStringList & strlist,
			      const TQString &caption = TQString::null,
			      const TQString &dontShowAgainName = TQString::null,
			      int options = Notify);

  /**
   * Enable all messages which have been turned off with the
   * @p dontShowAgainName feature.
   */
  static void enableAllMessages();

  /**
   * Re-enable a specific @p dontShowAgainName messages that had 
   * previously been turned off.
   * @see saveDontShowAgainYesNo()
   * @see saveDontShowAgainContinue()
   * @since 3.2
   */
  static void enableMessage(const TQString &dontShowAgainName);

  /**
   * Display an "About" dialog.
   *
   * @param parent  If @p parent is 0, then the message box becomes an
   *                application-global modal dialog box. If @p parent is a
   *                widget, the message box becomes modal relative to parent.
   * @param text    Message string.
   * @param caption Message box title. The application name is added to
   *                the title. The default title is i18n("About \<appname\>").
   * @param options  see OptionsType
   *
   *
   * Your program wants to show some general information about the application
   * like the authors's names and email addresses.
   *
   * The default button is "&OK".
   *
   *  NOTE: The ok button will always have the i18n'ed text '&OK'.
   */
  static void about(TQWidget *parent,
		    const TQString& text,
		    const TQString& caption = TQString::null,
                    int options = Notify);

    /**
     * Alternate method to show a messagebox:
     *
     * @param parent  If @p parent is 0, then the message box becomes an
     *                application-global modal dialog box. If @p parent is a
     *                widget, the message box becomes modal relative to parent.
     * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
     * @param text Message string.
     * @param caption Message box title.
     * @param buttonYes The text for the first button.
     *                  The default is i18n("&Yes").
     * @param buttonNo  The text for the second button.
     *                  The default is i18n("&No").
     * @param dontShowAskAgainName If provided, a checkbox is added with which
     *                further questions/informations can be turned off. If turned off
     *                all questions will be automatically answered with the
     *                last answer (either Yes or No), if the message box needs an answer.
     *                The string is used to lookup and store the setting
     *                in the applications config file.
     * @param options  see OptionsType
     * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
     *       and for Information, none is used.
     * @return a button code, as defined in KMessageBox.
     */
    static int messageBox( TQWidget *parent, DialogType type, const TQString &text,
                    const TQString &caption,
                    const KGuiItem &buttonYes,
                    const KGuiItem &buttonNo,
                    const TQString &dontShowAskAgainName,
                    int options = Notify);

    /**
     * Alternate method to show a messagebox:
     *
     * @param parent  If @p parent is 0, then the message box becomes an
     *                application-global modal dialog box. If @p parent is a
     *                widget, the message box becomes modal relative to parent.
     * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
     * @param text Message string.
     * @param caption Message box title.
     * @param buttonYes The text for the first button.
     *                  The default is i18n("&Yes").
     * @param buttonNo  The text for the second button.
     *                  The default is i18n("&No").
     * @param options  see OptionsType
     * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
     *       and for Information, none is used.
     * @return a button code, as defined in KMessageBox.
     */
    // KDE4 - merge with above?
    static int messageBox( TQWidget *parent, DialogType type, const TQString &text,
                    const TQString &caption = TQString::null,
                    const KGuiItem &buttonYes = KStdGuiItem::yes(),
                    const KGuiItem &buttonNo = KStdGuiItem::no(),
                    int options = Notify);

    /**
     * This function accepts the window id of the parent window, instead
     * of TQWidget*. It should be used only when necessary.
     * @since 3.2
     */
    static int messageBoxWId( WId parent_id, DialogType type, const TQString &text,
                    const TQString &caption = TQString::null,
                    const KGuiItem &buttonYes = KStdGuiItem::yes(),
                    const KGuiItem &buttonNo = KStdGuiItem::no(),
                    const TQString &dontShowAskAgainName = TQString::null,
                    int options = Notify);

    /**
     * Like messageBox
     *
     * Only for message boxes of type Information, Sorry or Error.
     *
     * This function will return immediately, the messagebox will be shown
     * once the application enters an event loop and no other messagebox
     * is being shown.
     *
     * Note that if the parent gets deleted, the messagebox will not be
     * shown.
     */
    static void queuedMessageBox( TQWidget *parent,
                    DialogType type, const TQString &text,
                    const TQString &caption,
                    int options );

    /**
     * This function accepts the window id of the parent window, instead
     * of TQWidget*. It should be used only when necessary.
     * @since 3.2
     */
    static void queuedMessageBoxWId( WId parent_id,
                    DialogType type, const TQString &text,
                    const TQString &caption,
                    int options );

    /**
     * @overload
     *
     * This is an overloaded member function, provided for convenience.
     * It behaves essentially like the above function.
     */
    static void queuedMessageBox( TQWidget *parent,
                    DialogType type, const TQString &text,
                    const TQString &caption = TQString::null );

    /**
     * This function accepts the window id of the parent window, instead
     * of TQWidget*. It should be used only when necessary.
     * @since 3.2
     */
    static void queuedMessageBoxWId( WId parent_id,
                    DialogType type, const TQString &text,
                    const TQString &caption = TQString::null );

    /**
     * @return true if the corresponding yes/no message box should be shown.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, true is always returned.
     * @param result is set to the result (Yes or No) that was chosen the last
     * time the message box was shown. Only meaningful, if the message box
     * should not be shown.
     * @since 3.2
     */
    static bool shouldBeShownYesNo(const TQString &dontShowAgainName,
                                   ButtonCode &result);
    /**
     * @return true if the corresponding continue/cancel message box should be
     * shown.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, true is always returned.
     * @since 3.2
     */
    static bool shouldBeShownContinue(const TQString &dontShowAgainName);

    /**
     * Save the fact that the yes/no message box should not be shown again.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method does nothing.
     * @param result the value (Yes or No) that should be used as the result
     * for the message box.
     * @since 3.2
     */
    static void saveDontShowAgainYesNo(const TQString &dontShowAgainName,
                                       ButtonCode result);

    /**
     * Save the fact that the continue/cancel message box should not be shown
     * again.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method does nothing.
     * @since 3.2
     */
    static void saveDontShowAgainContinue(const TQString &dontShowAgainName);

    /**
     * Use @p cfg for all settings related to the dontShowAgainName feature.
     * If @p cfg is 0 (default) TDEGlobal::config() will be used.
     * @since 3.2
     */    
    static void setDontShowAskAgainConfig(TDEConfig* cfg);

    /**
     * Create content and layout of a standard dialog
     *
     * @param dialog  The parent dialog base
     * @param icon    Which predefined icon the message box shall show.
     * @param text    Message string.
     * @param strlist List of strings to be written in the listbox.
     *                If the list is empty, it doesn't show any listbox
     * @param ask     The text of the checkbox. If empty none will be shown.
     * @param checkboxReturn The result of the checkbox. If it's initially
     *                true then the checkbox will be checked by default.
     * @param options  see OptionsType
     * @param details Detailed message string.
     * @since 3.3
     * @return A KDialogBase button code, not a KMessageBox button code,
     *         based on the buttonmask given to the constructor of the
     *         @p dialog (ie. will return KDialogBase::Yes [256] instead of 
     *         KMessageBox::Yes [3]). Will return KMessageBox::Cancel
     *         if the message box is queued for display instead of 
     *         exec()ed immediately or if the option NoExec is set.
     * @note   The @p dialog that is passed in is deleted by this
     *         function. Do not delete it yourself.
     */
    static int createKMessageBox(KDialogBase *dialog, TQMessageBox::Icon icon,
                             const TQString &text, const TQStringList &strlist,
                             const TQString &ask, bool *checkboxReturn,
                             int options, const TQString &details=TQString::null);

    /**
     * Create content and layout of a standard dialog
     *
     * @param dialog  The parent dialog base
     * @param icon    A TQPixmap containing the icon to be displayed in the
     *                dialog next to the text.
     * @param text    Message string.
     * @param strlist List of strings to be written in the listbox.
     *                If the list is empty, it doesn't show any listbox
     * @param ask     The text of the checkbox. If empty none will be shown.
     * @param checkboxReturn The result of the checkbox. If it's initially
     *                true then the checkbox will be checked by default.
     * @param options  see OptionsType
     * @param details Detailed message string.
     * @param notifyType The type of notification to send when this message
     *                is presentend.
     * @since 3.3
     * @return A KDialogBase button code, not a KMessageBox button code,
     *         based on the buttonmask given to the constructor of the
     *         @p dialog (ie. will return KDialogBase::Yes [256] instead of 
     *         KMessageBox::Yes [3]). Will return KMessageBox::Cancel
     *         if the message box is queued for display instead of 
     *         exec()ed immediately or if the option NoExec is set.
     * @note   The @p dialog that is passed in is deleted by this
     *         function. Do not delete it yourself.
     */
    static int createKMessageBox(KDialogBase *dialog, TQPixmap icon,
                             const TQString &text, const TQStringList &strlist,
                             const TQString &ask, bool *checkboxReturn,
                             int options, const TQString &details=TQString::null,
                             TQMessageBox::Icon notifyType=TQMessageBox::Information);

private:
    static TDEConfig* againConfig;
};

#endif
