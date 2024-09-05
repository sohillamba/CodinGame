const H = 3000;
const W = 7000;
const sizeFactor = 0.2;

var ground = [
    [0, 100], [1000, 500], [1500, 1500], [3000, 1000], [4000, 150], [5500, 150], [6999, 800]
];

const svgElement = document.getElementById('shuttle-visualization');
const tooltip = document.getElementById('tooltip');
const topFitnessDisplay = document.getElementById('top-fitness-display'); // Element to display top fitness values
let generations = [];
let currentGenerationIndex = 0;
let animationInterval = null;
let animationSpeed = 0.1; // Default speed in milliseconds
let overallTopChromosomes = []; // To store overall top 3 chromosomes

// Create a text element for displaying the generation number
const generationNumberText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
generationNumberText.setAttribute('x', 10); // X position
generationNumberText.setAttribute('y', 30); // Y position
generationNumberText.setAttribute('fill', 'red'); // Text color
generationNumberText.setAttribute('font-size', '24'); // Font size
// generationNumberText.textContent = 'Generation: 0'; // Default text
svgElement.appendChild(generationNumberText);

// Create a text element for displaying the fitness score of the first path
const fitnessScoreText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
fitnessScoreText.setAttribute('x', 10); // X position
fitnessScoreText.setAttribute('y', 60); // Y position (below the generation number)
fitnessScoreText.setAttribute('fill', 'yellow'); // Text color
fitnessScoreText.setAttribute('font-size', '20'); // Font size
svgElement.appendChild(fitnessScoreText);

// Set SVG background color
svgElement.style.backgroundColor = 'black';
svgElement.setAttribute('stroke-width', 0.5);

// Function to visualize the ground
function visualizeGround(color = 'red') {
    for (let i = 0; i < ground.length - 1; i++) {
        const [x1, y1] = ground[i];
        const [x2, y2] = ground[i + 1];
        const line = createLine(x1, H - y1, x2, H - y2, color);
        svgElement.appendChild(line);
    }
}

function processPaths(data) {
    if (!data || data.trim().length === 0) {
        console.error("No data found in the file.");
        return;
    }

    const lines = data.trim().split("\n");
    
    // Extract and process the ground coordinates from the first line
    const groundLine = lines.shift();
    const groundPoints = groundLine.trim().split(" ").map(coord => {
        const [x, y] = coord.split(",").map(Number);
        return [x, y];
    });
    
    ground = groundPoints;  // Update the global ground variable
    
    // Process the remaining lines as generations
    generations = lines.map((gen, genIndex) => {
        return gen.trim().split("|").map((chrom, chromIndex) => {
            const [fitnessPart, pathPart] = chrom.split(";");

            if (!fitnessPart || !pathPart) return null;

            const fitness = parseFloat(fitnessPart.replace("Fitness:", "").trim());
            if (isNaN(fitness)) return null;

            const path = pathPart.trim().split(" ").map((coord, coordIndex) => {
                const [x, y] = coord.split(",").map(Number);
                if (isNaN(x) || isNaN(y)) return null;
                return [x, 3000 - y];
            }).filter(point => point !== null);

            return { fitness, path };
        }).filter(chrom => chrom !== null);
    }).filter(gen => gen.length > 0);

    if (generations.length === 0) {
        console.error("No valid generations found.");
        return;
    }

    updateOverallTopChromosomes();
    visualizeGeneration(currentGenerationIndex);
}


function updateOverallTopChromosomes() {
    const allChromosomes = generations.flat();
    allChromosomes.sort((a, b) => b.fitness - a.fitness);
    overallTopChromosomes = allChromosomes.slice(0, 3);
}

// function visualizeGeneration(index) {
//     if (index < 0 || index >= generations.length) return;

//     svgElement.innerHTML = '';
//     visualizeGround();

//     // Update the generation number text
//     generationNumberText.textContent = `Generation: ${index + 1}`;
//     svgElement.appendChild(generationNumberText);

//     const currentGen = generations[index];

//     currentGen.forEach(chromosome => {
//         const isTop1 = chromosome === overallTopChromosomes[0];
//         const color = isTop1 ? 'blue' : getColorBasedOnFitness(chromosome.fitness);
//         visualizeShuttlePath(chromosome.path, chromosome.fitness, color, svgElement);
//     });

//     displayTopFitnessValues();

//     updateButtonState();
// }

function visualizeGeneration(index) {
    if (index < 0 || index >= generations.length) return;

    svgElement.innerHTML = '';
    visualizeGround();

    // Update the generation number text
    generationNumberText.textContent = `Generation: ${index + 1}`;
    svgElement.appendChild(generationNumberText);

    const currentGen = generations[index];

    // Update the fitness score text for the first path
    if (currentGen.length > 0) {
        const firstChromosome = currentGen[0];
        fitnessScoreText.textContent = `First Path Fitness: ${firstChromosome.fitness.toFixed(2)}`;
    } else {
        fitnessScoreText.textContent = `First Path Fitness: N/A`;
    }
    svgElement.appendChild(fitnessScoreText);

    currentGen.forEach(chromosome => {
        const isTop1 = chromosome === overallTopChromosomes[0];
        const color = isTop1 ? 'blue' : getColorBasedOnFitness(chromosome.fitness);
        visualizeShuttlePath(chromosome.path, chromosome.fitness, color, svgElement);
    });

    displayTopFitnessValues();

    updateButtonState();
}


function displayTopFitnessValues() {
    const topFitnessValues = overallTopChromosomes.map((chrom, i) => 
        `#${i + 1}: ${chrom.fitness.toFixed(2)}`
    ).join("<br>");

    topFitnessDisplay.innerHTML = `<strong>Overall Top 3 Fitness Values:</strong><br>${topFitnessValues}`;
}

function updateButtonState() {
    document.getElementById('prevGen').disabled = currentGenerationIndex === 0;
    document.getElementById('nextGen').disabled = currentGenerationIndex === generations.length - 1;
}

document.getElementById('fileInput').addEventListener('change', function(event) {
    const file = event.target.files[0];
    if (file) {
        const reader = new FileReader();
        reader.onload = function(e) {
            const content = e.target.result;
            console.log("File content loaded successfully");
            processPaths(content);
        };
        reader.readAsText(file);
    } else {
        console.error("No file selected");
    }
});

document.getElementById('prevGen').addEventListener('click', function() {
    if (currentGenerationIndex > 0) {
        currentGenerationIndex--;
        visualizeGeneration(currentGenerationIndex);
    }
});

document.getElementById('nextGen').addEventListener('click', function() {
    if (currentGenerationIndex < generations.length - 1) {
        currentGenerationIndex++;
        visualizeGeneration(currentGenerationIndex);
    }
});

document.getElementById('speed').addEventListener('input', function(event) {
    animationSpeed = parseInt(event.target.value, 10);
    if (animationInterval) {
        clearInterval(animationInterval);
        startAnimation();
    }
});

// function startAnimation() {
//     if (generations.length === 0) return;

//     animationInterval = setInterval(() => {
//         if (currentGenerationIndex < generations.length - 1) {
//             currentGenerationIndex++;
//             visualizeGeneration(currentGenerationIndex);
//         } else {
//             // Highlight the best path from all generations at the end of the animation
//             visualizeBestPath();
//             clearInterval(animationInterval);
//         }
//     }, animationSpeed);
// }

let lastRenderTime = 0;
function startAnimation() {
    if (generations.length === 0) return;
    function animate(time) {
        if (time - lastRenderTime > animationSpeed) {
            if (currentGenerationIndex < generations.length - 1) {
                currentGenerationIndex++;
                visualizeGeneration(currentGenerationIndex);
            } else {
                visualizeBestPath(); // Highlight the best path at the end
                return; // End the animation
            }
            lastRenderTime = time;
        }
        requestAnimationFrame(animate);
    }
    requestAnimationFrame(animate);
}


document.getElementById('startAnimation').addEventListener('click', function() {
    startAnimation();
});

function visualizeShuttlePath(path, fitness, color, svgElement) {
    path.forEach((point, i) => {
        if (i < path.length - 1) {
            const [x1, y1] = path[i];
            const [x2, y2] = path[i + 1];
            const line = createLine(x1, y1, x2, y2, color);
            svgElement.appendChild(line);
        }
    });
}

function createLine(x1, y1, x2, y2, color = 'black') {
    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', x1 * sizeFactor);
    line.setAttribute('y1', y1 * sizeFactor);
    line.setAttribute('x2', x2 * sizeFactor);
    line.setAttribute('y2', y2 * sizeFactor);
    line.setAttribute('stroke', color);
    return line;
}

function getColorBasedOnFitness(fitness) {
    // return rainbow(800, fitness);
    // Define the range for fitness values
    const minFitness = 0;
    const maxFitness = 300; // Adjust as needed for your data

    // Normalize the fitness value to a range of 0 to 1
    const normalizedFitness = (fitness - minFitness) / (maxFitness - minFitness);

    // Define gradient colors
    const startColor = { r: 255, g: 0, b: 0 }; // Red
    const endColor = { r: 0, g: 255, b: 0 }; // Green

    // Interpolate between start and end colors
    const r = Math.round(startColor.r + (endColor.r - startColor.r) * normalizedFitness);
    const g = Math.round(startColor.g + (endColor.g - startColor.g) * normalizedFitness);
    const b = Math.round(startColor.b + (endColor.b - startColor.b) * normalizedFitness);

    return `rgb(${r},${g},${b})`;
}

function visualizeBestPath() {
    if (overallTopChromosomes.length === 0) return;
    const bestPath = overallTopChromosomes[0];
    visualizeShuttlePath(bestPath.path, bestPath.fitness, 'gold', svgElement); // Highlight with unique color
}


function rainbow(numOfSteps, step) {
    // This function generates vibrant, "evenly spaced" colours (i.e. no clustering).
    // This is ideal for creating easily distinguishable vibrant markers in Google Maps and other apps.
    // Adam Cole, 2011-Sept-14
    // HSV to RBG adapted from: http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
    var r, g, b;
    var h = step / numOfSteps;
    var i = ~~(h * 6);
    var f = h * 6 - i;
    var q = 1 - f;
    switch (i % 6) {
        case 0: r = 1; g = f; b = 0; break;
        case 1: r = q; g = 1; b = 0; break;
        case 2: r = 0; g = 1; b = f; break;
        case 3: r = 0; g = q; b = 1; break;
        case 4: r = f; g = 0; b = 1; break;
        case 5: r = 1; g = 0; b = q; break;
    }
    var c = "#" +
        ("00" + (~ ~(r * 255)).toString(16)).slice(-2) +
        ("00" + (~ ~(g * 255)).toString(16)).slice(-2) +
        ("00" + (~ ~(b * 255)).toString(16)).slice(-2);
    return c;
}
