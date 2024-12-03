const container = document.querySelector(".container");
// generate 16 x 16 grid
let gridSize = 16;

function drawGrid()
{ 
    for (i = gridSize; i > 0; i--)
    {
        const rowDiv = document.createElement('div');
        rowDiv.classList.add("rowDiv");
        //rowDiv.textContent = "hi";
        container.appendChild(rowDiv);
    
        for (j = gridSize; j > 0; j--)
        {
            const sqDiv = document.createElement('div');
            sqDiv.classList.add("sqDiv");
            rowDiv.appendChild(sqDiv);
            sqDiv.addEventListener('mouseover', function(){
                sqDiv.classList.add("hovered");});
        }
}
}

drawGrid();
// New Grid prompt button
const btnNewGrid = document.querySelector("#NewGrid");
btnNewGrid.addEventListener('click', function()
{
    let newGridSize = parseInt(prompt("How wide a grid would you like, in squares?"));
    console.log(newGridSize);
    // input validation
    if (isNaN(newGridSize) == false && 1 <= newGridSize && newGridSize <= 100)
    {
        gridSize = newGridSize;
        container.innerHTML = "";
        drawGrid();
    }
    else alert("that's not a number between 1 and 100");
});


