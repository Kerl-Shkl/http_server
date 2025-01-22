
const test_arr = ["first", "second", "third"];
var selected = null;

function nameClicked()
{
  changeSelected(this);
}

function changeSelected(new_element)
{
  if (selected) {
    selected.classList.remove("selected");
  }
  selected = new_element;
  selected.classList.add("selected");
}

async function loadMenu()
{
  const response = await fetch("/api/note_names");
  const json = await response.json();
  console.log(json);

  var menu = document.getElementById("note_names");
  for (const note_name of json) {
    var new_item = document.createElement("a");
    new_item.appendChild(document.createTextNode(note_name));
    new_item.onclick = nameClicked;
    menu.appendChild(new_item);
  }
}
