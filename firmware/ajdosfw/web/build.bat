php ./zipmem/zipmem.php pack ./ajdos-setup.html ajdos_html
php ./zipmem/zipmem.php pack ./ajdos-setup.js ajdos_js
php ./zipmem/zipmem.php pack ./favico.ico ico
php ./zipmem/zipmem.php pack ./bootstrap.min.css bstrap_css
php ./zipmem/zipmem.php pack ./bootstrap.min.js bstrap_js
php ./zipmem/zipmem.php pack ./jquery-3.5.1.min.js jquery_js
copy /Y  .\ajdos-setup.html.h ..\res
copy /Y  .\ajdos-setup.js.h ..\res
copy /Y  .\favico.ico.h ..\res
copy /Y  .\bootstrap.min.css.h ..\res
copy /Y  .\bootstrap.min.js.h ..\res
copy /Y .\jquery-3.5.1.min.js.h ..\res