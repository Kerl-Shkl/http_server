
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

async function loadMenu()
{
  const response = await fetch("/api/note_names");
  const json = await response.json();
  console.log(json);
  menu_items.clear();

  var menu = document.getElementById("note_names");
  for (const element of json) {
    var new_item = document.createElement("a");
    new_item.appendChild(document.createTextNode(element.name));
    new_item.onclick = nameClicked;
    menu.appendChild(new_item);
    menu_items.set(new_item, element.id)
  }
}
