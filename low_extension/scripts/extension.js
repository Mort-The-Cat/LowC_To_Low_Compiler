'use strict'
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const vscode = require("vscode");
function activate(context){
    
    // context.subscriptions.push(vscode.languages.registerHoverProvider({ language: "Low", scheme: "file" }, new hover_1.ASMHoverProvider(Symbol_Documenter)));
    
}
exports.activate = activate;

function deactivate(){

}
exports.deactivate = deactivate;