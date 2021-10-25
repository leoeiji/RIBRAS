T="$(date +%s)"

# Delete every .root file first
cd ROOT
rm *.root
cd ..

# Interval for Current1
iCurrent1=100
fCurrent1=700

# Interval for Current2
iCurrent2=0
fCurrent2=0

# Primary beam position (cm)
Primary_pos=-116

# Primary beam energy (MeV)
Primary_e=$(echo "scale=2; 3500/100.0" | bc)

# Target definition
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
Target_z=148
# 148
# 478

# Step/10
Step=1

# Changing analyse macro
sed -i "s/^    for(Current1.*/    for(Current1 = $(echo "scale=2; $iCurrent1/10.0" | bc); Current1<=$(echo "scale=2; $fCurrent1/10.0" | bc); Current1 ++)/" macros/analise4.mac
sed -i "s/^        for(Current2.*/        for(Current2 = $(echo "scale=2; $iCurrent2/10.0" | bc); Current2<=$(echo "scale=2; $fCurrent2/10.0" | bc); Current2 ++)/" macros/analise4.mac

# Initializing simulation
echo 'Initializing simulation:'
echo ' '
echo "Current 1 interval: $(echo "scale=2; $iCurrent1/10.0" | bc)A to $(echo "scale=2; $fCurrent1/10.0" | bc)A"
echo "Current 2 interval: $(echo "scale=2; $iCurrent2/10.0" | bc)A to $(echo "scale=2; $fCurrent2/10.0" | bc)A"
echo "Primary beam position: $Primary_pos m"
echo "Primary beam energy: $Primary_e"
echo "Target position: $Target_z m"      
echo "Step: $(echo "scale=2; $Step/10.0" | bc) A"                      
sleep 5

# Running simulation with a interval of 1.0
for ((x1 = $iCurrent1 ; x1 <= $fCurrent1 ; x1+=10)); 
do
    # Getting value of current 1
    r1=$(echo "scale=2; $x1/10.0" | bc)
    r2i=$(echo "scale=2; $iCurrent2/10.0" | bc)
    r2f=$(echo "scale=2; $fCurrent2/10.0" | bc)

    # Create macro ready for loop
    echo -e "/control/verbose 0 \n/run/verbose 0 \n/vis/disable \n/det/field 1 \n/det/target/material G4_POLYETHYLENE \n/det/target/Z $Target_Z \n/det/target/A $Target_A \n/det/target/width 1.89e-3 \n/det/target/pos 0. 0. $Target_z \n/det/primary/energy $Primary_e MeV \n/det/primary/Z $Primary_Z \n/det/primary/A $Primary_A \n/det/primary/pos 0. 0. $Primary_pos \n/det/recoil/A $Recoil_A \n/det/recoil/Z $Recoil_Z \n/det/ejectile/A $Ejectile_A \n/det/ejectile/Z $Ejectile_Z" "\n/det/currentValue $r1" "\n/det/currentValue2 {current}" "\n/det/update" "\n/run/beamOn 1000" > tmp.mac

    # Create looping macro
    echo -e "/control/loop tmp.mac current $r2i $r2f 1.0" > looping.mac

    # Run looping macro
    ./arquivobinario looping.mac
done

# Analyse everything
cd macros
root -l <<-EOF
.x analise4.mac
.q
EOF

# Now, getting currents
lines=(`cat "Current.txt"`)
fCurrent1=${lines[0]}
fCurrent2=${lines[1]}
iCurrent1=${lines[2]}
iCurrent2=${lines[3]}

if [ $fCurrent1 -ne -1000 ]
then
    # Delete everything for the next run
    cd ..
    cd ROOT
    rm *.root
    cd ..

    # Changing analyse macro
    sed -i "s/^    for(Current1.*/    for(Current1 = $(echo "scale=2; $iCurrent1/10.0" | bc); Current1<=$(echo "scale=2; $fCurrent1/10.0" | bc); Current1 += $(echo "scale=2; $Step/10.0" | bc))/" macros/analise3.mac
    sed -i "s/^        for(Current2.*/        for(Current2 = $(echo "scale=2; $iCurrent2/10.0" | bc); Current2<=$(echo "scale=2; $fCurrent2/10.0" | bc); Current2 += $(echo "scale=2; $Step/10.0" | bc))/" macros/analise3.mac

    # Running simulation with a interval of 0.2
    for ((x1 = $iCurrent1 ; x1 <= $fCurrent1 ; x1+=$Step)); 
    do
        # Getting value of current 1
        r1=$(echo "scale=2; $x1/10.0" | bc)
        r2i=$(echo "scale=2; $iCurrent2/10.0" | bc)
        r2f=$(echo "scale=2; $fCurrent2/10.0" | bc)

        # Create macro ready for loop
        echo -e "/control/verbose 0 \n/run/verbose 0 \n/vis/disable \n/det/field 1 \n/det/target/material G4_POLYETHYLENE \n/det/target/Z $Target_Z \n/det/target/A $Target_A \n/det/target/width 1.89e-3 \n/det/target/pos 0. 0. $Target_z \n/det/primary/energy $Primary_e MeV \n/det/primary/Z $Primary_Z \n/det/primary/A $Primary_A \n/det/primary/pos 0. 0. $Primary_pos \n/det/recoil/A $Recoil_A \n/det/recoil/Z $Recoil_Z \n/det/ejectile/A $Ejectile_A \n/det/ejectile/Z $Ejectile_Z" "\n/det/currentValue $r1" "\n/det/currentValue2 {current}" "\n/det/update" "\n/run/beamOn 1000" > tmp.mac

        # Create looping macro
        echo -e "/control/loop tmp.mac current $r2i $r2f $(echo "scale=2; $Step/10.0" | bc)" > looping.mac

        # Run looping macro
        ./arquivobinario looping.mac
    done

    # Delete everything when done
    rm looping.mac
    rm tmp.mac
    echo 'Simulation finished'

    # Then, analyse everything
    echo ' '
    echo 'Now, analysing...'

    # Running macro
    cd macros

root -l <<-EOF
.x analise3.mac
.q
EOF

    cd ..
    cd ROOT
    rm *.root
    cd ..
else
echo " "
echo "-----------------------------"
echo "-- Couldn't find any focus --"
echo "-----------------------------"
echo " "
cd ..
cd ROOT
rm *.root
cd ..
fi

# Print simulation time
T="$(($(date +%s)-T))"
echo "Duration: $T s"