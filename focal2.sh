T="$(date +%s)"

# Delete every .root file first
cd ROOT
rm *.root
cd ..

# Interval for Current
iCurrent1=150
fCurrent1=250

# Primary beam position (cm)
Primary_pos=-115

# Primary beam energy (MeV)
Primary_e=35

# Target partcles
Target_Z=2
Target_A=4

# Primary beam particles
Primary_Z=3
Primary_A=8

# Recoil particles
Recoil_Z=2
Recoil_A=4

# Ejectile particles
Ejectile_Z=3
Ejectile_A=8

# Target z position (cm)
######################################################################
# Second Solenoid's magnetic field starts at 267cm and ends at 335cm #
######################################################################
Target_z=148

# Changing analyse macro
sed -i "s/^    for(Current1.*/    for(Current1 = $((iCurrent1/10)); Current1<=$((fCurrent1/10)); Current1 ++)/" macros/analise1.mac

# Initializing simulation
echo 'Initializing simulation:'
echo ' '
echo "Current 1 interval: $((iCurrent1/10))A to $((fCurrent1/10))A"
echo "Primary beam position: $Primary_pos m"
echo "Primary beam energy: $Primary_e"
echo "Target position: $Target_z m"                            
sleep 5

# Running simulation with a interval of 1.0

# Getting value of current 1
r1i=$(echo "scale=2; $iCurrent1/10.0" | bc)
r1f=$(echo "scale=2; $fCurrent1/10.0" | bc)

# Create macro ready for loop
echo -e "/control/verbose 0 \n/run/verbose 0 \n/vis/disable \n/det/field 1 \n/det/target/Z $Target_Z \n/det/target/A $Target_A \n/det/target/width 0.001 \n/det/target/pos 0. 0. $Target_z \n/det/primary/energy $Primary_e MeV \n/det/primary/Z $Primary_Z \n/det/primary/A $Primary_A \n/det/primary/pos 0. 0. $Primary_pos \n/det/recoil/A $Recoil_A \n/det/recoil/Z $Recoil_Z \n/det/ejectile/A $Ejectile_A \n/det/ejectile/Z $Ejectile_Z" "\n/det/currentValue {current}" "\n/det/update" "\n/run/beamOn 100" > tmp.mac

# Create looping macro
echo -e "/control/loop tmp.mac current $r1i $r1f 0.1" > looping.mac

# Run looping macro
./arquivobinario looping.mac

# Analyse everything
cd macros
root -l <<-EOF
.x analise1.mac
.q
EOF

# Now, getting currents
lines=(`cat "Current.txt"`)
fCurrent1=${lines[0]}
iCurrent1=${lines[1]}

# Print simulation time
T="$(($(date +%s)-T))"
echo "Duration: $T s"