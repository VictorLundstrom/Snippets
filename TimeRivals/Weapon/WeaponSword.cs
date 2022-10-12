using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponSword : WeaponBaseClass
{
    protected override void Awake()
    {
        base.Awake();
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject.layer == LayerMask.NameToLayer("Default")) //If sword is colliding with the environment.
        {
            return;
        }

        if (ShouldApplyForce == false && _PlayerMovementBehavior.CurrentlyAttacking) //If player started an Attack and hit a physical object or Player
        {
            ShouldApplyForce = true;
            gameObject.layer = DefaultWeaponLayer; //Change to default layer to stop further collisions vs players/physical objects

            GetComponent<BoxCollider>().enabled = false; //Temporarily Disable collider after hitting, Re-Enable after Attack animation has finshed in PlayerMovementBehavior.

            Vector3 dir = (collision.GetContact(0).point - transform.root.GetChild(0).position).normalized;



            if (collision.gameObject.layer == LayerMask.NameToLayer("PhysicalObject")) //If we hit a physical object in the world
            {
                collision.rigidbody.AddForce(dir * Objectforce, ForceMode.Impulse);

                //
                switch (_PlayerMovementBehavior.CurrAttack)
                {
                    case AttackIndexes.SWORD:
                    case AttackIndexes.BAT:
                        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bat/slag", transform.position);
                        break;
                }
                //
                return;
            }
            collision.rigidbody.AddForce(dir * TotalForce, ForceMode.Impulse);

            GameObject targetPlayer = collision.transform.root.GetChild(0).gameObject; //if we hit a player

            targetPlayer.GetComponent<HealthSystem>().Damage(_Damage, playerID);

            Debug.Log("Hit " + targetPlayer.name);

            //////////////////////////////////////

            CurrDurability -= _DurabilityCost;

            if (CurrDurability <= 0)
            {
                ShouldDestroy = true;
            }

            //
            switch (_PlayerMovementBehavior.CurrAttack)
            {
                case AttackIndexes.BAT:
                    FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bat/slag", transform.position);
                    break;
            }
            //
        }
    }

    public override void ResetVariables()
    {
        ShouldApplyForce = false;
        _BoxCollider.enabled = true;
    }
}
