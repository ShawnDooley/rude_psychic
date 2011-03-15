/* This is a modified version of the original Psychic plugin that 
 * comes standard with the Pidgin application. Most of the original
 * code was written by Christopher O'Brien. I changed the output 
 * from only showing up in your local window as "a disturbance in 
 * the force" to a immediate (rude) message sent to the person that
 * started typing to you.
 */


#include "internal.h"

#include "account.h"
#include "blist.h"
#include "conversation.h"
#include "debug.h"
#include "signals.h"
#include "status.h"
#include "version.h"
#include "privacy.h"

#include "util.h"
#include "request.h"
#include "notify.h"

#include "plugin.h"
#include "pluginpref.h"
#include "prefs.h"


#define PLUGIN_ID       "core-wazutiman-psychic"
#define PLUGIN_NAME     N_("Rude Psychic Mode-test")
#define PLUGIN_VERSION  "1.0.2"
#define PLUGIN_SUMMARY  N_("Will offend people trying to talk to you.")
#define PLUGIN_DESC     N_("Sends a rude message to people when they start " \
			   "typing a new IM to you. Works with" \
			   " AIM, ICQ, XMPP, Sametime, and Yahoo!")
#define PLUGIN_AUTHOR   "Shawn Dooley <wazutiman@gmail.com>"
#define PLUGIN_WEBSITE "www.wazutiman.com"

#define PREFS_BASE    "/plugins/core/psychic"
#define PREF_BUDDIES  PREFS_BASE "/buddies_only"
#define PREF_STATUS   PREFS_BASE "/activate_online"
#define PREF_RAISE    PREFS_BASE "/raise_conv"

#define PREF_RUDE     PREFS_BASE "/rude_messages"



/*Returns a pointer to a random phrase*/
static char * rude_phrase(void)
{

	char* opts[5];
	
	opts[0] = "What the (*&%) do you want?";
	opts[1] = "I don't care?";
	opts[2] = "La la la! I'm not listening!";
	opts[3] = "Why do you think I care?";	
	opts[4] = "Who the #$%^ are you?";
		
	srand(time(NULL));
	return opts[(rand() %  5)];	
}

static char * random_phrase(void)
{
	char* opts[5];
	
	opts[0] = "Hi, how are you doing?";
	opts[1] = "So, what are you up to?";
	opts[2] = "Fine thanks, and you?";
	opts[3] = "I see you!";	
	opts[4] = "I have no idea.";
		
	srand(time(NULL));
	return opts[(rand() %  5)];	
}


static void
buddy_typing_cb(PurpleAccount *acct, const char *name, void *data) 
{

  PurpleConversation *gconv = NULL;
  PurpleConvIm *imconv = NULL;
  PurpleBuddy *imbuddy = NULL;
  const char *msg = NULL;
  if(purple_prefs_get_bool(PREF_STATUS) &&
     ! purple_status_is_available(purple_account_get_active_status(acct))) 
	{
    purple_debug_info("psychic", "not available, doing nothing\n");
    return;
  }

  if(purple_prefs_get_bool(PREF_BUDDIES) &&
     ! purple_find_buddy(acct, name)) {
    purple_debug_info("psychic", "not in blist, doing nothing\n");
    return;
  }

  if(FALSE == purple_privacy_check(acct, name)) 
	{
    purple_debug_info("psychic", "user %s is blocked\n", name);
    return;
  }

  gconv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, name, acct);
  if(! gconv) 
	{
    purple_debug_info("psychic", "no previous conversation exists\n");
    gconv = purple_conversation_new(PURPLE_CONV_TYPE_IM, acct, name);

    if(purple_prefs_get_bool(PREF_RAISE)) 
    {
      purple_conversation_present(gconv);
    }
    


/* get the im information */    
    imconv = purple_conversation_get_im_data(gconv);
    


  imbuddy = purple_find_buddy(acct, name);
  
  msg = purple_blist_node_get_string(&imbuddy->node, "rude_msg");
//Check to see fi there is a rude_msg stored in the buddy list xml
  if(msg)
  {
    purple_conv_im_send(imconv, msg);
  }
  else
  {
    //If there isnt a custome message for the user, use whatever the
    //default is set to (rude or regular)
    if(purple_prefs_get_bool(PREF_RUDE))
      purple_conv_im_send(imconv, rude_phrase());
    else
      purple_conv_im_send(imconv, random_phrase());
	     
  }


    /* Necessary because we may be creating a new conversation window. */
    purple_conv_im_set_typing_state(PURPLE_CONV_IM(gconv), PURPLE_TYPING);
  }
}


static void
dont_do_it_cb(PurpleBlistNode *node, const char *note)
{
//Dont save
}

static void
do_it_cb(PurpleBlistNode *node, const char *note)
{
	//Save new "rude_msg" tag in buddylist xml
	purple_blist_node_set_string(node, "rude_msg", note);
}


static void
rude_message_cb(PurpleBlistNode *node, gpointer data)
{
	const char *note;

	note = purple_blist_node_get_string(node, "rude_msg");

	purple_request_input(node, _("Message"),
					   _("Enter your message Below..."),
					   NULL,
					   note, TRUE, FALSE, "html",
					   _("Save"), G_CALLBACK(do_it_cb),
					   _("Cancel"), G_CALLBACK(dont_do_it_cb),
					   NULL, NULL, NULL,
					   node);
}


static void
buddynote_rude_menu_cb(PurpleBlistNode *node, GList **m)
{
	PurpleMenuAction *bna = NULL;

	if (purple_blist_node_get_flags(node) & PURPLE_BLIST_NODE_FLAG_NO_SAVE)
		return;

	*m = g_list_append(*m, bna);
	bna = purple_menu_action_new(_("Custom rude psychic message."), PURPLE_CALLBACK(rude_message_cb), NULL, NULL);
	*m = g_list_append(*m, bna);
}


static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin) 
{

  PurplePluginPrefFrame *frame;
  PurplePluginPref *pref;

  frame = purple_plugin_pref_frame_new();

  pref = purple_plugin_pref_new_with_name(PREF_BUDDIES);
  purple_plugin_pref_set_label(pref, _("Only enable for users on"
				     " the buddy list"));
  purple_plugin_pref_frame_add(frame, pref);

  pref = purple_plugin_pref_new_with_name(PREF_STATUS);
  purple_plugin_pref_set_label(pref, _("Disable when away"));
  purple_plugin_pref_frame_add(frame, pref);


  pref = purple_plugin_pref_new_with_name(PREF_RAISE);
  purple_plugin_pref_set_label(pref, _("Raise psychic conversations"));
  purple_plugin_pref_frame_add(frame, pref);

//
  pref = purple_plugin_pref_new_with_name(PREF_RUDE);
  purple_plugin_pref_set_label(pref, _("Use rude messages"));
  purple_plugin_pref_frame_add(frame, pref);
//


  return frame;
}


static gboolean
plugin_load(PurplePlugin *plugin) 
{

	//call buddy_typing_cb when a buddy starts typing to you. 
  purple_signal_connect(purple_conversations_get_handle(), 
  "buddy-typing", plugin, PURPLE_CALLBACK(buddy_typing_cb), NULL);

	//
	//Connect to buddylist menu signal
	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu", plugin,	PURPLE_CALLBACK(buddynote_rude_menu_cb), NULL);


  return TRUE;
}


static PurplePluginUiInfo prefs_info = 
{
  get_plugin_pref_frame,
  0,    /* page_num (Reserved) */
  NULL, /* frame (Reserved) */

  /* padding */
  NULL,
  NULL,
  NULL,
  NULL
};


static PurplePluginInfo info = 
{
  PURPLE_PLUGIN_MAGIC,
  PURPLE_MAJOR_VERSION,
  PURPLE_MINOR_VERSION,
  PURPLE_PLUGIN_STANDARD,   /**< type */
  NULL,                   /**< ui_requirement */
  0,                      /**< flags */
  NULL,                   /**< dependencies */
  PURPLE_PRIORITY_DEFAULT,  /**< priority */

  PLUGIN_ID,              /**< id */
  PLUGIN_NAME,            /**< name */
  PLUGIN_VERSION,        /**< version */
  PLUGIN_SUMMARY,         /**< summary */
  PLUGIN_DESC,            /**< description */
  PLUGIN_AUTHOR,          /**< author */
  PLUGIN_WEBSITE,           /**< homepage */

  plugin_load,            /**< load */
  NULL,                   /**< unload */
  NULL,                   /**< destroy */

  NULL,                   /**< ui_info */
  NULL,                   /**< extra_info */
  &prefs_info,            /**< prefs_info */
  NULL,                   /**< actions */

  /* padding */
  NULL,
  NULL,
  NULL,
  NULL
};


static void
init_plugin(PurplePlugin *plugin) 
{
  purple_prefs_add_none(PREFS_BASE);
	purple_prefs_add_bool(PREF_RUDE, FALSE);  
	purple_prefs_add_bool(PREF_BUDDIES, FALSE);
  purple_prefs_add_bool(PREF_STATUS, TRUE);
  

}


PURPLE_INIT_PLUGIN(psychic, init_plugin, info)
