PROGRAM = gtkcalc

DOC_MODULE=gtkcalc

ROOT = ""

# Sources Files ... ugly ?
SRC = 	"src"/*.c 

# Paquets à linker
PAQUETS = 	guile-1.8 \
			glib-2.0 \
			gtk+-3.0 \
			gmodule-export-2.0 \
			gobject-2.0 
#			libgda-5.0 NOT NEEDED YET ... AND MAYBE NEVER !!!
			
PKGS = `pkg-config --cflags --libs $(PAQUETS)`


# Configuration normale
CONFIG = -DDATA=\"`pwd`/Data\" \
		 -DUI_FILE=\"`pwd`/Data/calc.ui\"

# Configuration pour l'installation
CONFIG_INSTALL = -DDATA=\"/usr/share/$(PROGRAM)/Data\" \
				 -DUI_FILE=\"/usr/share/$(PROGRAM)/Data/calc.ui\"

# Arguments du compilateur
OPTS = -g

# Commande du compilateur
CC = gcc

.PHONY : clean
clean:
	@rm -v -fr src/*~ src/*.o
	

# Cible basique
.PHONY : all
all:
	$(CC) $(OPTS) $(SRC) -o Build/$(PROGRAM) $(PKGS) $(CONFIG)
	
.PHONY : doc
doc:
	gtkdoc-scan --module=$(DOC_MODULE) --source-dir=./src --output-dir=Doc
	cd Doc && gtkdoc-mkdb --module=$(DOC_MODULE) --output-format=xml && cd html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MODULE)-docs.xml && cd .. &&  gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=html
	CC=gcc  CFLAGS=`pkg-config --cflags guile-1.8 gtk+-3.0 gobject-2.0`  LDFLAGS=`pkg-config --libs guile-1.8 gtk+-3.0 gobject-2.0`  RUN=exec  cd Doc && gtkdoc-scangobj --module=$(DOC_MODULE)

# Installe sur le système !
.PHONY : install
install : clean
	@rm -v -fr $(PROGRAM)
	# Compilation du programme
	$(CC) $(OPTS) $(SRC) -o Build/$(PROGRAM) $(PKGS) $(CONFIG_INSTALL)
	
	# Création du répertoire du programme
	@mkdir -p "$(ROOT)/usr/share/$(PROGRAM)"
	
	# Copie des données du programme
	@cp -R ./Data/ "$(ROOT)/usr/share/$(PROGRAM)/Data"
	
	# Modification des permissions des fichiers 
	@chmod -R u+rw "$(ROOT)/usr/share/$(PROGRAM)"
	
	# Installation du .desktop
	@mkdir -p "$(ROOT)/usr/share/applications"
	@cp `pwd`/DesktopIntegration/$(PROGRAM).desktop "$(ROOT)/usr/share/applications/$(PROGRAM).desktop"
	@chmod u+x "$(ROOT)/usr/share/applications/$(PROGRAM).desktop"
	
	# Installation des icones
	@mkdir -p "$(ROOT)/usr/share/icons/hicolor/48x48/apps"
	@mkdir -p "$(ROOT)/usr/share/icons/hicolor/64x64/apps"
	@mkdir -p "$(ROOT)/usr/share/icons/hicolor/128x128/apps"
	@mkdir -p "$(ROOT)/usr/share/icons/hicolor/scalable/apps"
	@cp `pwd`/DesktopIntegration/48.png "$(ROOT)/usr/share/icons/hicolor/48x48/apps/$(PROGRAM).png"
	@cp `pwd`/DesktopIntegration/64.png "$(ROOT)/usr/share/icons/hicolor/64x64/apps/$(PROGRAM).png"
	@cp `pwd`/DesktopIntegration/128.png "$(ROOT)/usr/share/icons/hicolor/128x128/apps/$(PROGRAM).png"
	@cp `pwd`/DesktopIntegration/Icon.svg "$(ROOT)/usr/share/icons/hicolor/scalable/apps/$(PROGRAM).svg"
	
	# Installation de la man page
	@mkdir -p "$(ROOT)/usr/share/man/man6"
	@cp `pwd`/DesktopIntegration/$(PROGRAM).6.gz "$(ROOT)/usr/share/man/man6/$(PROGRAM).6.gz"
	
	# Déplacement du programme dans /usr/bin
	@mkdir -p "$(ROOT)/usr/bin"
	@mv Build/$(PROGRAM) "$(ROOT)/usr/bin/$(PROGRAM)"
	
# Désinstalle
.PHONY : uninstall
uninstall: clean
	# Supression des données du programme
	@rm -v -fr -r "/usr/share/$(PROGRAM)"
	
	# Supression du programme lui même
	@rm -v -fr "/usr/bin/$(PROGRAM)"
	
	# Supression de l'intégration au desktop
	@rm -v -fr "/usr/share/applications/$(PROGRAM).desktop"
	@rm -v -fr "/usr/share/icons/hicolor/48x48/apps/$(PROGRAM).png"
	@rm -v -fr "/usr/share/icons/hicolor/64x64/apps/$(PROGRAM).png"
	@rm -v -fr "/usr/share/icons/hicolor/128x128/apps/$(PROGRAM).png"
	@rm -v -fr "/usr/share/icons/hicolor/scalable/apps/$(PROGRAM).svg"
	@rm -v -fr "/usr/share/man/man6/$(PROGRAM).6.gz"
	
	@echo "Désinstallé avec succès"

