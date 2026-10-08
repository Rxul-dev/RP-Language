-- App "RP": abre los archivos .rp y los ejecuta en la Terminal con el compilador.

on run
	-- Sin archivos no hay nada que ejecutar. El instalador abre la app una vez
	-- así para que macOS confíe en ella y muestre el logo en los archivos .rp.
end run

on open archivos
	set compilador to POSIX path of (path to me) & "Contents/Resources/rp"
	repeat with archivo in archivos
		set ruta to POSIX path of archivo
		set carpeta to do shell script "dirname " & quoted form of ruta
		set nombre to do shell script "basename " & quoted form of ruta
		tell application "Terminal"
			activate
			do script "clear; cd " & quoted form of carpeta & " && " & quoted form of compilador & " " & quoted form of nombre
		end tell
	end repeat
end open
