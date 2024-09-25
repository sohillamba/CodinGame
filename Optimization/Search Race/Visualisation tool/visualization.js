const H = 9000; // Adjusted height of the plane
const W = 16000; // Adjusted width of the plane
const sizeFactor = 0.075; // Adjusted scaling factor for larger SVG canvas
const checkpointRadius = 600; // Fixed radius for checkpoints

let generations = [];
let currentGenerationIndex = 0;
let animationSpeed = 100; // Default animation speed in milliseconds
let overallTopChromosomes = [];
let checkpoints = [];

// References to the SVG and other HTML elements
const svgElement = document.getElementById('svgCanvas');
const tooltip = document.getElementById('tooltip');
const topFitnessDisplay = document.getElementById('topFitnessDisplay'); 

// Create a text element to display the generation number
const generationNumberText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
generationNumberText.setAttribute('x', 10);
generationNumberText.setAttribute('y', 30);
generationNumberText.setAttribute('fill', 'red');
generationNumberText.setAttribute('font-size', '24');
svgElement.appendChild(generationNumberText);

// Create a text element to display fitness score of the first path
const fitnessScoreText = document.createElementNS('http://www.w3.org/2000/svg', 'text');
fitnessScoreText.setAttribute('x', 10);
fitnessScoreText.setAttribute('y', 60);
fitnessScoreText.setAttribute('fill', 'yellow');
fitnessScoreText.setAttribute('font-size', '20');
svgElement.appendChild(fitnessScoreText);

// Set SVG background color
svgElement.style.backgroundColor = 'black';
svgElement.setAttribute('stroke-width', 0.5);

// Function to visualize the checkpoints
function visualizeCheckpoints() {
    checkpoints.forEach(([x, y]) => {
        const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
        circle.setAttribute('cx', x * sizeFactor);
        circle.setAttribute('cy', y * sizeFactor); // Adjust for Y coordinate flip
        circle.setAttribute('r', checkpointRadius * sizeFactor); // Apply sizeFactor to radius
        circle.setAttribute('stroke', 'yellow');
        circle.setAttribute('stroke-width', '2');
        circle.setAttribute('fill', 'none');
        svgElement.appendChild(circle);
    });
}

// Function to process the file input (paths and checkpoints)
function processPaths(data) {
    if (!data || data.trim().length === 0) {
        console.error("No data found in the file.");
        return;
    }

    const lines = data.trim().split("\n");

    // Extract and process the checkpoints from the first line
    const checkpointsLine = lines.shift();
    checkpoints = checkpointsLine.trim().split(" ").map(coord => {
        const [x, y] = coord.split(",").map(Number);
        return [x, y];
    });

    // Process the remaining lines as generations
    generations = lines.map((gen, genIndex) => {
        return gen.trim().split("|").map((chrom, chromIndex) => {
            const [fitnessPart, pathPart] = chrom.split(";");

            if (!fitnessPart || !pathPart) return null;

            const fitness = parseFloat(fitnessPart.replace("Fitness:", "").trim());
            if (isNaN(fitness)) return null;

            const path = pathPart.trim().split(" ").map((coord) => {
                const [x, y] = coord.split(",").map(Number);
                if (isNaN(x) || isNaN(y)) return null;
                return [x, y];
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

function visualizeGeneration(index) {
    if (index < 0 || index >= generations.length) return;

    svgElement.innerHTML = ''; // Clear previous visualization
    visualizeCheckpoints(); // Draw the checkpoints

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
    if (fitness > 90) return 'green';
    if (fitness > 70) return 'yellow';
    return 'red';
}

function visualizeBestPath() {
    const bestChromosome = overallTopChromosomes[0];
    visualizeShuttlePath(bestChromosome.path, bestChromosome.fitness, 'blue', svgElement);
}
