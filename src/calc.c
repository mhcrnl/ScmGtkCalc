/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * calc.c
 * Copyright (C) 2012 Babakask <lopezaliaume@gmail.com>
 * 
 * ScmCalcGtk is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ScmCalcGtk is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "calc.h"

/**
 * Définition du type pour GType
 */
G_DEFINE_TYPE (ScmCalc, scmcalc, G_TYPE_OBJECT)

static void scmcalc_finalize (GObject *self);
static void scmcalc_class_init (ScmCalcClass* klass);
static void scmcalc_init (ScmCalc* self);
static GtkWidget* scmcalc_create_window (ScmCalc *self);
static void scmcalc_create_layout (ScmCalc *self, const gchar* path);
static SCM wrapper_body_proc (gpointer data);
static SCM wrapper_handler_proc (gpointer data, SCM key, SCM param);
static void scmcalc_set_code_style (GtkTextView* code);
static void scmcalc_delete_layout (ScmCalc *self);

/**
 * scmcalc_class_init:
 * @klass : #ScmCalcClass
 *
 * Initialise la classe  
 *
 */
static void
scmcalc_class_init (ScmCalcClass *klass) 
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = scmcalc_finalize;
	g_return_if_fail (klass != NULL);
}

/**
 * scmcalc_init:
 * @self : Un objet #ScmCalc qui va être initialisé
 *
 * Initialise l'objet
 */
static void
scmcalc_init (ScmCalc *self) 
{
	scm_init_guile ();
	
	scm_c_define_gsubr ("carre", 1, 0, 0, scm_carre);
	
	/*
	 * Chargement :
	 *		- dictionnaire
	 * 		- suites
	 * 		- conversions 
	 *		- operations 
	 */
	scm_c_primitive_load (DATA "/Scheme/utils.scm");
	scm_c_primitive_load (DATA "/Scheme/dico.scm");
	scm_c_primitive_load (DATA "/Scheme/suites.scm");
	scm_c_primitive_load (DATA "/Scheme/convertions.scm");
	scm_c_primitive_load (DATA "/Scheme/operations.scm");
	
	self->window = scmcalc_create_window (self);
	
	scmcalc_create_layout (self, DATA "/layout.ui");
	
	gtk_widget_show (self->window);
	
	g_return_if_fail (self != NULL);
}

/**
 * scmcalc_finalize:
 * @self_object
 *
 * Détruit l'objet
 * 
 */
static void 
scmcalc_finalize (GObject *self_object)
{
	ScmCalc* self = SCM_CALC (self_object);
	
}

/**
 * scmcalc_new:
 *
 * Creates a newly allocated ScmCalc
 *
 * Returns: #ScmCalc*
 */
ScmCalc*
scmcalc_new ()
{
	g_type_init ();
	return g_object_new (TYPE_SCM_CALC, NULL);
}

/**
 * scmcalc_free:
 * @scmcalc: the #ScmCalc to delete, can be NULL
 *
 * Deletes a #ScmCalc
 *
 */
void 
scmcalc_free (ScmCalc* scmcalc)
{
	if (scmcalc != NULL) {
		g_object_unref (scmcalc);
		scmcalc = NULL;
	}
}

static void
scmcalc_set_code_style (GtkTextView* code)
{
	PangoFontDescription* font = pango_font_description_from_string ("Monospace 24");
	gtk_widget_modify_font (GTK_WIDGET (code), font);
	pango_font_description_free (font);
}

/**
 * scmcalc_create_window:
 * @self : le #ScmCalc qui doit recevoir la nouvelle fenêtre
 *
 * Crée une nouvelle fenêtre et met à jour les attributs du #ScmCalc 
 * en conséquence.
 *
 * Returns: Une GtkWidget (window) fraîchement allouée :D
 */
static GtkWidget*
scmcalc_create_window (ScmCalc *self)
{
	GtkWidget *window;
	GtkBuilder *builder;
	GtkTextView* h;
	GError* error = NULL;
	const gchar *widget_missing = _("Widget \"%s\" is missing in file %s.");
	
	#define UI_FILE DATA "/calc_test.ui"
	
	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical (_("Couldn't load builder file: %s"), error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, self);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        if (!window)
        {
                g_critical ( widget_missing,
				"window",
				UI_FILE);
		}
	
	self->global_layout = GTK_BOX (gtk_builder_get_object (builder, "global_layout"));
        if (!self->global_layout)
        {
                g_critical ( widget_missing,
				"window",
				UI_FILE);
		}
	g_printf ("Finish load window \n");

	g_object_unref (builder);
	
	gtk_window_set_icon_from_file (GTK_WINDOW(window), DATA "/icon.png", &error);
	
	if (error != NULL) {
		g_critical (_("Couldn't set application icon : %s !\n"),
				error->message);
	}
	
	return window;
}

/**
 * scmcalc_create_layout:
 *
 * Crée le layout de la fenetre
 */
static void
scmcalc_create_layout (ScmCalc *self, const gchar* UI_INTERN)
{
	GtkBuilder *builder;
	GtkTextView* h;
	GError* error = NULL;
	GtkWidget* layout = NULL;
	const gchar *widget_missing = _("Widget \"%s\" is missing in file %s.");
	
	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_INTERN, &error))
	{
		g_critical (_("Couldn't load builder file: %s"), error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, self);

	self->sortie = GTK_LABEL (gtk_builder_get_object (builder, "sortie"));
        if (!self->sortie)
        {
                g_critical (widget_missing,
				"sortie",
				UI_INTERN);
		}

	self->code = GTK_TEXT_VIEW (gtk_builder_get_object (builder, "code"));
        if (!self->code)
        {
                g_critical (widget_missing,
				"code",
				UI_INTERN);
		}
	scmcalc_set_code_style (self->code);

	h = GTK_TEXT_VIEW (gtk_builder_get_object (builder, "historique"));
        if (!h)
        {
                g_critical (widget_missing,
				"historique",
				UI_INTERN);
		}

	self->prec_cmd = GTK_LABEL (gtk_builder_get_object (builder, "prec_cmd"));
        if (!self->prec_cmd)
        {
                g_critical (widget_missing,
				"prec_cmd",
				UI_INTERN);
		}

	self->historique = gtk_text_view_get_buffer (h);
	
	layout = GTK_WIDGET (gtk_builder_get_object (builder, "calculette"));
        if (!layout)
        {
                g_critical (widget_missing,
				"calculette",
				UI_INTERN);
		}
	
	self->current_layout = layout;
	gtk_box_pack_end (self->global_layout, GTK_WIDGET (layout), TRUE, TRUE, 0);
	g_object_unref (builder);
}

/** 
 * scmcalc_delete_layout:
 *
 * Détruit les éléments Gtk Contenus dans 
 * le layout actuel
 */
static void
scmcalc_delete_layout (ScmCalc* self) 
{
	gtk_widget_destroy (self->current_layout);
}
/**
 * scmcalc_load_new_ui:
 * 
 * Supprime l'ancien layout (current_layout) 
 * et en charge un nouveau selon le fichier 
 * passé en argument
 */
void
scmcalc_load_new_ui (ScmCalc* self, const gchar* path)
{
	scmcalc_delete_layout (self);
	scmcalc_create_layout (self, path);
}

/**
 * scmcalc_disp:
 * @self : #ScmCalc to set
 * @action : the string to display (without the parenthesis)
 *
 * Displays the action on the #GtkEntry with parenthesis around
 *
 */
void 
scmcalc_disp (ScmCalc *self, const gchar* action)
{
	
	gchar* cmd = g_strdup_printf("(%s)", action);
	
	GtkTextBuffer* buf = gtk_text_view_get_buffer (self->code);
	GtkTextIter insert;
	
	GtkTextMark* mark_insert = gtk_text_buffer_get_insert (buf);
	gtk_text_buffer_get_iter_at_mark (buf, &insert, mark_insert);
	
	gtk_text_buffer_insert (buf, &insert, cmd, -1);
	
	mark_insert = gtk_text_buffer_get_insert (buf);
	gtk_text_buffer_get_iter_at_mark (buf, &insert, mark_insert);
	
	gint cp = gtk_text_iter_get_offset (&insert);
	
	gtk_text_buffer_get_iter_at_offset (buf, &insert, cp - 1);
	
	gtk_text_buffer_place_cursor (buf, &insert);

	g_free (cmd);
}


static SCM
wrapper_body_proc (gpointer data)
{
	gchar* cmd = (gchar*) data;
	return scm_c_eval_string (cmd);
}

static SCM
wrapper_handler_proc (gpointer data, SCM key, SCM param)
{
	ScmCalc* self = SCM_CALC (data);
	return param;
}

/**
 * scmcalc_execute:
 * @self : #ScmCalc to infer with
 * @ation : the action to be executed
 *
 * Execute the action passed
 *
 */
void 
scmcalc_execute (ScmCalc* self, const gchar *action) 
{
	SCM result;
	SCM rep;

	gtk_label_set_label (self->prec_cmd, action);
	
	result = scm_c_catch (SCM_BOOL_T,
                    wrapper_body_proc, (gpointer) action,
                    wrapper_handler_proc, (gpointer) self,
                    NULL, NULL);
	
	/**
	 * Si c'est un nombre on le sauvegarde 
	 */
	if (scm_is_number (result)) {

		SCM t = scm_number_to_string (result, scm_from_int (10));
		
		SCM append_func_symbol = scm_c_lookup("precedent-add");
 		SCM append_func = scm_variable_ref (append_func_symbol);
		scm_call_1 (append_func, result);
	}
	
	{
		SCM format_symbol = scm_c_lookup("format");
	 	SCM format = scm_variable_ref (format_symbol);
		SCM string = scm_call_3 (format, SCM_BOOL_F, scm_from_locale_string ("~a"), result);
		gtk_label_set_label (self->sortie, scm_to_locale_string (string));
	}
}

/**
 * scmcalc_execute_save:
 * @self : #ScmCalc to infer
 * @ation : the action to be executed
 *
 * The same as scmcalc_execute() except that it
 * saves the command in the command-history with 
 * scmcalc_add_historique() 
 *
 */
void 
scmcalc_execute_save (ScmCalc* self, const gchar *action)
{
	scmcalc_execute (self, action);
	scmcalc_add_historique (self, action);
}

/**
 * scmcalc_add_historique:
 * @self : #ScmCalc to infer
 * @ation : the action to be saved
 *
 * Saves the command in the command history 
 *
 */
void 
scmcalc_add_historique (ScmCalc* self, const gchar *text)
{
	gint lines = gtk_text_buffer_get_line_count (self->historique);
	GtkTextBuffer * code_buf = gtk_text_view_get_buffer (self->code);
	GtkTextIter iter, start, end, code_start, code_end;
	
	if (lines >= 10) {
		gtk_text_buffer_get_iter_at_line (self->historique, &start, 0);
		gtk_text_buffer_get_iter_at_line (self->historique, &end, 1);
		gtk_text_buffer_delete (self->historique, &start, &end);
	}
	
	gtk_text_buffer_get_start_iter (code_buf, &code_start);
	gtk_text_buffer_get_end_iter (code_buf, &code_end);

	gtk_text_buffer_get_iter_at_line (self->historique, &iter, lines);
	gtk_text_buffer_insert (self->historique, &iter, gtk_text_buffer_get_text (code_buf, &code_start, &code_end, FALSE), -1);
	gtk_text_buffer_insert (self->historique, &iter, "\n", -1);
	
	gtk_text_buffer_get_end_iter (self->historique, &iter);
	gtk_text_buffer_place_cursor (self->historique, &iter);
}

/**
 * scmcalc_disp_string:
 * @self : #ScmCalc to infer
 * @ation : the string to be added
 *
 * Insert the string litteral into the command prompt
 *
 */
void
scmcalc_disp_string (ScmCalc* self, const gchar* action)
{
	GtkTextBuffer* buf = gtk_text_view_get_buffer (self->code);
	GtkTextIter insert;
	
	GtkTextMark* mark_insert = gtk_text_buffer_get_insert (buf);
	gtk_text_buffer_get_iter_at_mark (buf, &insert, mark_insert);
	
	gtk_text_buffer_insert (buf, &insert, action, -1);
}
