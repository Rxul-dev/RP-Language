// Extensión de VS Code para RP: agrega el comando "Ejecutar programa RP" (botón ▶ y tecla F5).
const vscode = require("vscode");
const fs = require("fs");
const path = require("path");

// Busca rp (o rp.exe) en la carpeta del archivo y en las de arriba, salvo que se configure la ruta
function buscarCompilador(archivo) {
  const configurado = vscode.workspace.getConfiguration("rp").get("rutaCompilador");
  if (configurado) return configurado;
  const nombre = process.platform === "win32" ? "rp.exe" : "rp";
  let carpeta = path.dirname(archivo);
  for (;;) {
    const candidato = path.join(carpeta, nombre);
    if (fs.existsSync(candidato)) return candidato;
    const padre = path.dirname(carpeta);
    if (padre === carpeta) return null;
    carpeta = padre;
  }
}

function activate(context) {
  const comando = vscode.commands.registerCommand("rp.ejecutar", async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== "rp") {
      vscode.window.showWarningMessage("Abra un archivo .rp para ejecutarlo.");
      return;
    }
    await editor.document.save();
    const archivo = editor.document.fileName;
    const compilador = buscarCompilador(archivo);
    if (!compilador) {
      vscode.window.showErrorMessage(
        "No se encontró el compilador de RP. Compílelo con 'make' (Mac) o 'compilar.bat' (Windows), " +
        "o indique su ruta en la configuración 'rp.rutaCompilador'."
      );
      return;
    }
    const terminal = vscode.window.terminals.find((t) => t.name === "RP") || vscode.window.createTerminal("RP");
    terminal.show();
    const linea = `"${compilador}" "${archivo}"`;
    // En PowerShell (terminal por defecto de Windows) un programa entre comillas se ejecuta con &
    terminal.sendText(process.platform === "win32" ? `& ${linea}` : linea);
  });
  context.subscriptions.push(comando);
}

module.exports = { activate, deactivate() {} };
