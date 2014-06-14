For Documentation Translators
=============================

Make translation templates
--------------------------

To make translation templates, run the following command in ``sphinx`` directory::

    $ make gettext

Then, *.pot files are created in ``sphinx/locale``.


Translate into your language
----------------------------

We show an example structure of locale directory as follows::

    sphinx/
     |- locale
         |- android
         |   |- android.pot
         |   |- ja.po
         |   |- de.po
         |   |- fr.po
         |   |- ...
         |
         |- audio
         |   |- audio.pot
         |   |- ja.po
         |   |- ...
         |
        ...

In this example, ``android.pot`` and ``audio.pot`` are template files, and ``*.po`` files are
translation files of each template. If you want to create your own translation file,
please copy a template file as `lang`.po in the same directory.

You can use Translation Memory (TM) to edit PO files. We introduce some TM softwares:

* PO edit
* OmegaT
* Launchpad (web interface)


Build translated documentation
------------------------------

To make translated documentation, run the following commands::

    $ make compile-po
    $ LANGUAGE=lang make html

``lang`` is a language name used in the beginning of PO files.
If you specify ``LANGUAGE=ja``, ``ja.po`` on each directory are used to translate.
