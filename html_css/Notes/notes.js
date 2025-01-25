
var selected = null;
var menu_items = new Map();

function nameClicked()
{
    changeSelected(this);
    getNote(menu_items.get(this))
}

function changeSelected(new_element)
{
    if (selected) {
        selected.classList.remove("selected");
    } else {
    }
    if (new_element) {
        document.getElementById("delete_note").disabled = false;
    } else {
        document.getElementById("delete_note").disabled = true;
    }
    selected = new_element;
    selected.classList.add("selected");
}

async function getNote(id)
{
    const response = await fetch("/api/note_body", {
        headers: {
            "id": id
        }
    });
    var article = document.getElementById("note_body");
    const text = await response.text();
    article.innerHTML = text;
    renderMathInElement(article, {
        // customised options
        // • auto-render specific keys, e.g.:
        delimiters: [
            {left: '$$', right: '$$', display: true},
            {left: '$', right: '$', display: false},
            {left: '\\(', right: '\\)', display: false},
            {left: '\\[', right: '\\]', display: true}
        ],
        // • rendering keys, e.g.:
        throwOnError : false
    });
}

function handleLoad()
{
    loadMenu();
    setupAddNoteWindow();
    setupSureDeleteWindow();
}

async function loadMenu()
{
    const response = await fetch("/api/note_names");
    const json = await response.json();
    console.log(json);
    menu_items.clear();
    var menu = document.getElementById("note_names");
    menu.replaceChildren();

    for (const element of json) {
        var new_item = document.createElement("a");
        new_item.appendChild(document.createTextNode(element.name));
        new_item.onclick = nameClicked;
        menu.appendChild(new_item);
        menu_items.set(new_item, element.id)
    }
}

function setupAddNoteWindow()
{
    var add_window = document.getElementById("add_note_window");
    var open_btn = document.getElementById("add_note_btn");
    var close_btn = document.getElementById("close_add_note");
    open_btn.onclick = function() {
        add_window.style.display = "block";
    }
    close_btn.onclick = function() {
        closeAddNoteWindow();
    }
    window.onclick=function(event) {
        if (event.target == add_window) {
            closeAddNoteWindow();
        }
    }
}

function setupSureDeleteWindow()
{
    var delete_window = document.getElementById("sure_delete_window");
    var open_btn = document.getElementById("delete_note");
    var close_btn = document.getElementById("close_delete_note");
    var note_name = document.getElementById("delete_note_name")
    if (!selected) {
        open_btn.disabled = true;
    }
    open_btn.onclick = function() {
        delete_window.style.display = "block";
        note_name.innerHTML = selected.text;
        console.log(selected.text);
    }
    close_btn.onclick = function() {
        closeDeleteWindow();
    }
    window.onclick=function(event) {
        if (event.target == delete_window) {
            closeDeleteWindow();
        }
    }
}

function sendNote() {
    var note_name = document.getElementById("input_nname").value;
    if (!note_name) {
        alert("У заметки должно быть имя");
        return;
    }
    var section_name = document.getElementById("input_sname").value;
    var finput = document.getElementById("file_input");
    let reader = new FileReader();
    reader.onload = async (e) => {
        console.log(e.target.result);
        const response = await fetch("/api/add_note", {
            method: "post",
            headers: {
                'Content-Type': 'text/html',
                'name': note_name,
                'section_name': section_name
            },
            body: e.target.result
        })
        if (!response.ok) {
            alert("Во время отправки заметки произошла ошибка " , response.status , response.statusText);
        } else {
            loadMenu();
        }
    }
    reader.onerror = (e) => alert(e.target.error.name);
    reader.readAsText(finput.files[0]);
    closeAddNoteWindow();
}

async function deleteNote() {
    var note_id = menu_items.get(selected);
    const response = await fetch("/api/delete_note", {
        method: "delete",
        headers: {
            'id': note_id,
        },
    })
    if (!response.ok) {
        alert("Во время удаления заметки произошла ошибка" , response.status , response.statusText);
    } else {
        loadMenu();
    }
    closeDeleteWindow();
}

function closeAddNoteWindow() {
    document.getElementById("input_nname").value = "";
    document.getElementById("input_sname").value = "";
    document.getElementById("file_input").value = "";
    var add_window = document.getElementById("add_note_window");
    add_window.style.display = "none";
}

function closeDeleteWindow() {
    document.getElementById("delete_note_name").innerHTML = "";
    var delete_window = document.getElementById("sure_delete_window");
    delete_window.style.display = "none";
}
